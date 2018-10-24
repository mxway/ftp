#include "stdafx.h"
#include "Command.h"
#include <event2/bufferevent.h>
#include "StringUtil.h"

void	parseRealPath(LPCTSTR prmPath)
{
	CStringUtil	str = prmPath;
	str.ReplaceStr(TEXT("\\"), TEXT("/"));
	vector<CStringUtil>	pathArray;
	str.SplitString(pathArray, TEXT("/"));
	CStringUtil	result;
	if (prmPath[0] == TEXT('/'))
	{
		result.Append(TEXT("/"));
	}
	vector<CStringUtil>	realPathArray;
	for (int i = 0; i < pathArray.size(); i++)
	{
		if (pathArray[i] == TEXT(".."))
		{
			if (realPathArray.size() == 0)
			{
				break;
			}
			realPathArray.pop_back();
		}
		else if (pathArray[i] == TEXT("."))
		{
			continue;
		}
		else if ()
		{
			realPathArray.push_back(pathArray[i]);
		}
	}
	for (int i = 0; i < realPathArray.size(); i++)
	{
		if (i > 0)
		{
			result.Append(TEXT("/"));
		}
		result.Append(realPathArray[i]);
	}
	printf("%s\n", result.GetString());
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
	std::string	str = "257 \"";
	str.append(conn->m_workDir).append("\" is current work directory\r\n");
	bufferevent_write(conn->m_bev, str.c_str(), str.length());
}

void CCwdCommand::processCommand(FTP_Connection_s *conn)
{

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

void CQuitCommand::processCommand(FTP_Connection_s *conn)
{
	std::string		str = "220 goodbye\r\n";
	send(conn->m_fd, str.c_str(), str.length(), 0);
}