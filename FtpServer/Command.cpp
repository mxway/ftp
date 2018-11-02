#include "stdafx.h"
#include "Command.h"
#include <event2/bufferevent.h>
#include "StringUtil.h"
#include <io.h>
#include <IPHlpApi.h>
#include "DataServer.h"

#pragma  comment(lib,"iphlpapi.lib")
#ifdef _UNICODE
#define IsFileExists _waccess
#else
#define IsFileExists _access
#endif

CStringUtil	GetRealPath(LPCTSTR prmPath)
{
	CStringUtil	str = prmPath;
	str.ReplaceStr(TEXT("\\"), TEXT("/"));
	vector<CStringUtil>	pathArray;
	str.SplitString(pathArray, TEXT("/"));
	CStringUtil	result;
	vector<CStringUtil>	realPathArray;
	if (str.StartWith(TEXT("/")))
	{
		result.Append("/");
	}
	for (size_t i = 0; i < pathArray.size(); i++)
	{
		//处理相对路径的引用
		if (pathArray[i].CompareNChar(TEXT(".."), 2) == 0)
		{
			//出现了如"/../plugin"或"/plugin/../../123.txt"这样的非法引用
			if (realPathArray.size() == 0)
			{
				return TEXT("");
			}
			realPathArray.pop_back();
		}
		else if (pathArray[i] != TEXT("."))
		{
			//"./"代表当前路径，若在字符串中出现，忽略该字符
			realPathArray.push_back(pathArray[i]);
		}
	}
	for (size_t i = 0; i < realPathArray.size(); i++)
	{
		if (i != 0)
		{
			result.Append(TEXT("/"));
		}
		result.Append(realPathArray[i]);
	}
	return result;
}

CStringUtil	GetParentPath(LPCTSTR prmPath)
{
	CStringUtil	str = GetRealPath(prmPath);
	int	index = str.RFindString(TEXT("/"));
	if (index == -1)
	{
		return TEXT("");
	}
	if(index==0 && str.GetLength()==1)
	{
		return TEXT("");
	}
	else if(index==0 && str.GetLength()>1)
	{
		return TEXT("/");
	}
	return str.Left(index);
}

BOOL	CreateDirRecursive(const CStringUtil &path)
{
	vector<CStringUtil>		pathArray;
	CStringUtil				tmpPath = path;
	tmpPath.ReplaceStr(TEXT("\\"),TEXT("/"));
	tmpPath.SplitString(pathArray,TEXT("/"));
	if(pathArray.size()<=1)
	{
		return TRUE;
	}
	CStringUtil		curDir = pathArray[0];
	for(size_t i=1;i<pathArray.size();i++)
	{
		curDir.Append(TEXT("/")).Append(pathArray[i]);
		if(IsFileExists(curDir.GetString(),0)!=0)
		{
			//文件夹不存在，需要创建
			if(CreateDirectory(curDir.GetString(),NULL)==0)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

void CPwdCommand::processCommand(FTP_Connection_s *conn)
{
	int err = ::GetLastError();
	char	errorMsg[] = "530 Please login with USER and pass\r\n";
	if (conn->m_status != FTP_LOGIN)
	{
		bufferevent_write(conn->m_bev, errorMsg, strlen(errorMsg));
		return;
	}
	CStringUtil	str = "257 \"";
	str.Append(conn->m_workDir).Append("\" is current work directory\r\n");
	bufferevent_write(conn->m_bev, str.GetString(), str.GetLength());
}

void CCwdCommand::processCommand(FTP_Connection_s *conn)
{
	CStringUtil		param(conn->m_param, 0,conn->m_paramLen);
	CStringUtil		realFileName;
	if (param.StartWith(TEXT("/")) || param.StartWith(TEXT("\\")) )
	{
		realFileName = GetRealPath(param.GetString());
	}
	else
	{
		CStringUtil	tmpPath = conn->m_workDir + TEXT("/") + param;
		realFileName = GetRealPath(tmpPath.GetString());
	}
	if (realFileName.GetLength() == 0)
	{
		char	msg[] = "550 paramaters error\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		localFileName = conn->m_rootDir + TEXT("/") + realFileName;
	//测试文件在磁盘上是否存在。
	CStringUtil		realLocalFileName = GetRealPath(localFileName.GetString());
	if (IsFileExists(realLocalFileName.GetString(), 0) != 0)
	{
		char	msg[] = "530 invalid path\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	conn->m_workDir = realFileName;
	char		okMsg[] = "220 change current work path\r\n";
	bufferevent_write(conn->m_bev,okMsg,strlen(okMsg));
}

void CFtpLoginCommand::processCommand(FTP_Connection_s *conn)
{
	if (strncmp(conn->m_command, "user", conn->m_commandLen) == 0)
	{
		std::string		str = "331 password required\r\n";
		bufferevent_write(conn->m_bev, str.c_str(), str.length());
	}
	else if (strncmp(conn->m_command, "pass", conn->m_commandLen) == 0
		&& strncmp(conn->m_param, "mxl123", conn->m_paramLen) == 0)
	{
		conn->m_status = FTP_LOGIN;
		std::string	str = "230 Logged on\r\n";
		bufferevent_write(conn->m_bev, str.c_str(), str.length());
	}
	else
	{
		std::string	str = "530 Login or password incorrect!\r\n";
		bufferevent_write(conn->m_bev, str.c_str(), str.length());
	}
}

void CCDupCommand::processCommand(FTP_Connection_s *conn)
{
	CStringUtil		parentDir = GetParentPath(conn->m_workDir.GetString());
	if(parentDir.GetLength()==0)
	{
		char	msg[] = "440 / is current work dir\r\n";
		bufferevent_write(conn->m_bev,msg,strlen(msg));
		return;
	}
	conn->m_workDir = parentDir;
	char		okMsg[] = "200 change to parent dir\r\n";
	bufferevent_write(conn->m_bev,okMsg,strlen(okMsg));
}

void CNoopCommand::processCommand(FTP_Connection_s *conn)
{
	char	msg[] = "200 ok\r\n";
	bufferevent_write(conn->m_bev,msg,strlen(msg));
}

void CPasvCommand::processCommand(FTP_Connection_s *conn)
{
	struct sockaddr		addr;
	int	   sockLen = sizeof(struct sockaddr);
	::getsockname(conn->m_fd,&addr,&sockLen);
	struct  sockaddr_in		*sin = (struct sockaddr_in*)&addr;

	int port = this->getDataListenPort();
	if(conn->m_dataServer)
	{
		conn->m_dataServer->StopThread();
		conn->m_dataServer->WaitThread();
		delete conn->m_dataServer;
	}
	conn->m_dataServer = new CDataServer(sin->sin_addr.s_addr,port,conn);
	conn->m_dataServer->CreateThread(TEXT("数据线程"));
	conn->m_dataServer->StartThread();
}

int	CPasvCommand::getDataListenPort()
{
	srand((unsigned int)time(NULL));
	//随机产生[50000-60000]之间的一个端口作为监听端口
	int		port = rand()%10000+50000;
	while(isTcpPortInUse(port))
	{
		port = rand()%10000+50000;
	}
	int		tmpVal = rand()%10000+50000;
	return port;
}

BOOL CPasvCommand::isTcpPortInUse(int port)
{
	DWORD		nPort = ntohs(port);
	PMIB_TCPTABLE_OWNER_PID	pTcpTable(NULL);
	DWORD		dwSize = 0;
	::GetExtendedTcpTable(pTcpTable,&dwSize,TRUE,AF_INET,TCP_TABLE_OWNER_PID_ALL,0);
	pTcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(dwSize);
	::GetExtendedTcpTable(pTcpTable,&dwSize,TRUE,AF_INET,TCP_TABLE_OWNER_PID_ALL,0);
	for(DWORD i=0;i<pTcpTable->dwNumEntries;i++)
	{
		if(nPort == pTcpTable->table[i].dwLocalPort)
		{
			free(pTcpTable);
			return TRUE;
		}
	}
	free(pTcpTable);
	return FALSE;
}

void CTypeCommand::processCommand(FTP_Connection_s *conn)
{
	char	msg[] = "220 ok\r\n";
	bufferevent_write(conn->m_bev, msg, strlen(msg));
}

void CListCommand::processCommand(FTP_Connection_s *conn)
{
	CStringUtil		param(conn->m_param, 0, conn->m_paramLen);
	CStringUtil		realFileName;
	if (param.StartWith(TEXT("/")) || param.StartWith(TEXT("\\")))
	{
		realFileName = GetRealPath(param.GetString());
	}
	else
	{
		CStringUtil	tmpPath = conn->m_workDir + TEXT("/") + param;
		realFileName = GetRealPath(tmpPath.GetString());
	}
	if (realFileName.GetLength() == 0)
	{
		char	msg[] = "550 paramaters error\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		localFileName = conn->m_rootDir + TEXT("/") + realFileName;
	conn->m_dataServer->list(localFileName);
	//测试文件在磁盘上是否存在。
	conn->m_dataServer->StopThread();
	conn->m_dataServer->WaitThread();
	delete conn->m_dataServer;
	conn->m_dataServer = NULL;
}

void CRetrCommand::processCommand(FTP_Connection_s *conn)
{
	if(conn->m_paramLen==0)
	{
		char		msg[] = "533 invalid parameter\r\n";
		bufferevent_write(conn->m_bev,msg,strlen(msg));
		return;
	}
	CStringUtil		param(conn->m_param, 0, conn->m_paramLen);
	CStringUtil		realFileName;
	if (param.StartWith(TEXT("/")) || param.StartWith(TEXT("\\")))
	{
		realFileName = GetRealPath(param.GetString());
	}
	else
	{
		CStringUtil	tmpPath = conn->m_workDir + TEXT("/") + param;
		realFileName = GetRealPath(tmpPath.GetString());
	}
	if (realFileName.GetLength() == 0)
	{
		char	msg[] = "550 paramaters error\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		localFileName = conn->m_rootDir + TEXT("/") + realFileName;
	if(IsFileExists(localFileName.GetString(),0)!=0)
	{
		char	msg[] = "530 invalid filename\r\n";
		bufferevent_write(conn->m_bev,msg,strlen(msg));
		return;
	}
	conn->m_dataServer->retrFile(localFileName);
	conn->m_dataServer->StopThread();
	conn->m_dataServer->WaitThread();
	delete conn->m_dataServer;
	conn->m_dataServer = NULL;
}

void CStoreFileCommand::processCommand(FTP_Connection_s *conn)
{
	if(conn->m_paramLen==0)
	{
		char		msg[] = "533 invalid parameter\r\n";
		bufferevent_write(conn->m_bev,msg,strlen(msg));
		return;
	}
	CStringUtil		param(conn->m_param, 0, conn->m_paramLen);
	CStringUtil		realFileName;
	if (param.StartWith(TEXT("/")) || param.StartWith(TEXT("\\")))
	{
		realFileName = GetRealPath(param.GetString());
	}
	else
	{
		CStringUtil	tmpPath = conn->m_workDir + TEXT("/") + param;
		realFileName = GetRealPath(tmpPath.GetString());
	}
	if (realFileName.GetLength() == 0)
	{
		char	msg[] = "550 paramaters error\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		localFileName = conn->m_rootDir + TEXT("/") + realFileName;
	CStringUtil		folderPath = GetParentPath(localFileName.GetString());
	if(folderPath.GetLength()==0)
	{
		char	msg[] = "551 invalid filename\r\n";
		bufferevent_write(conn->m_bev,msg,strlen(msg));
		return;
	}
	CreateDirRecursive(folderPath);
	conn->m_dataServer->storeFile(localFileName);
}

void CMkDirCommand::processCommand(FTP_Connection_s *conn)
{
	if (conn->m_paramLen == 0)
	{
		char		msg[] = "533 invalid parameter\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		param(conn->m_param, 0, conn->m_paramLen);
	CStringUtil		realFileName;
	if (param.StartWith(TEXT("/")) || param.StartWith(TEXT("\\")))
	{
		realFileName = GetRealPath(param.GetString());
	}
	else
	{
		CStringUtil	tmpPath = conn->m_workDir + TEXT("/") + param;
		realFileName = GetRealPath(tmpPath.GetString());
	}
	if (realFileName.GetLength() == 0)
	{
		char	msg[] = "550 paramaters error\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		localFileName = conn->m_rootDir + TEXT("/") + realFileName;
	BOOL ret = CreateDirRecursive(localFileName);
	if (ret)
	{
		char	okMsg[] = "220 dir has been created\r\n";
		bufferevent_write(conn->m_bev, okMsg, strlen(okMsg));
	}
	else
	{
		char	errMsg[] = "530 can not create dir\r\n";
		bufferevent_write(conn->m_bev, errMsg, strlen(errMsg));
	}
}

void CRmdCommand::processCommand(FTP_Connection_s *conn)
{
	CStringUtil	    param;
	if(conn->m_paramLen==0)
	{
		param = "/";
	}
	else
	{
		param = CStringUtil(conn->m_param,0,conn->m_paramLen);
	}
	CStringUtil		realFileName;
	if (param.StartWith(TEXT("/")) || param.StartWith(TEXT("\\")))
	{
		realFileName = GetRealPath(param.GetString());
	}
	else
	{
		CStringUtil	tmpPath = conn->m_workDir + TEXT("/") + param;
		realFileName = GetRealPath(tmpPath.GetString());
	}
	if (realFileName.GetLength() == 0)
	{
		char	msg[] = "550 paramaters error\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		localFileName = conn->m_rootDir + TEXT("/") + realFileName;
	BOOL ret = RemoveDirectory(localFileName.GetString());
	if (ret != 0)
	{
		char	okMsg[] = "220 directory has been removed\r\n";
		bufferevent_write(conn->m_bev, okMsg, strlen(okMsg));
	}
	else
	{
		char	errMsg[] = "530 directory may not empty\r\n";
		bufferevent_write(conn->m_bev, errMsg, strlen(errMsg));
	}
}

void CDeleCommand::processCommand(FTP_Connection_s *conn)
{
	if (conn->m_paramLen == 0)
	{
		char		msg[] = "450 invalid parameter\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		param(conn->m_param, 0, conn->m_paramLen);
	CStringUtil		realFileName;
	if (param.StartWith(TEXT("/")) || param.StartWith(TEXT("\\")))
	{
		realFileName = GetRealPath(param.GetString());
	}
	else
	{
		CStringUtil	tmpPath = conn->m_workDir + TEXT("/") + param;
		realFileName = GetRealPath(tmpPath.GetString());
	}
	if (realFileName.GetLength() == 0)
	{
		char	msg[] = "500 paramaters error\r\n";
		bufferevent_write(conn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil		localFileName = conn->m_rootDir + TEXT("/") + realFileName;
	DWORD		dwAttr = ::GetFileAttributes(localFileName.GetString());
	if(dwAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		char		errMsg[] = "502 can not delete folder\r\n";
		bufferevent_write(conn->m_bev,errMsg,strlen(errMsg));
		return;
	}
	BOOL ret = ::DeleteFile(localFileName.GetString());
	if( ret != 0)
	{
		char	msg[] = "250 delete file success\r\n";
		bufferevent_write(conn->m_bev,msg,strlen(msg));
	}
	else
	{
		char	msg[] = "504 can not delete file\r\n";
		bufferevent_write(conn->m_bev,msg,strlen(msg));
	}
}

void CQuitCommand::processCommand(FTP_Connection_s *conn)
{
	std::string		str = "220 goodbye\r\n";
	send(conn->m_fd, str.c_str(), str.length(), 0);
}