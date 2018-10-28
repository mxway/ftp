#include "stdafx.h"
#include "Command.h"
#include <event2/bufferevent.h>
#include "StringUtil.h"
#include <io.h>

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

void CPwdCommand::processCommand(FTP_Connection_s *conn)
{
	char	fileName[] = "F:/software/CTK-master/../1.jpg";
	WIN32_FIND_DATA	fileData;
	HANDLE	fileHandle = ::FindFirstFile(fileName, &fileData);
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
	//if (param.StartWith())
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

void CQuitCommand::processCommand(FTP_Connection_s *conn)
{
	std::string		str = "220 goodbye\r\n";
	send(conn->m_fd, str.c_str(), str.length(), 0);
}