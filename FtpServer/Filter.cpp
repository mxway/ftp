#include "stdafx.h"
#include "Filter.h"

CBaseFilter::CBaseFilter(CBaseFilter *filter /* = NULL */)
	:m_next(filter)
{

}

BOOL CBaseFilter::filter(FTP_Connection_s *conn)
{
	if(strncmp(conn->m_command,"quit",conn->m_commandLen)==0)
	{
		return TRUE;
	}
	return FALSE;
}

CLogStatusFilter::CLogStatusFilter(CBaseFilter *next)
	:CBaseFilter(next)
{

}

BOOL CLogStatusFilter::filter(FTP_Connection_s *conn)
{
	BOOL	ret = CBaseFilter::filter(conn);
	if(ret == TRUE)
	{
		return ret;
	}
	if(conn->m_status < FTP_LOGIN)
	{
		if(strncmp(conn->m_command,"user",conn->m_commandLen)!=0
			&& strncmp(conn->m_command,"pass",conn->m_commandLen)!=0)
		{
			char		errMsg[] = "530 please use user and pass to login\r\n";
			bufferevent_write(conn->m_bev,errMsg,strlen(errMsg));
			return FALSE;
		}
	}

	if(m_next != NULL)
	{
			return m_next->filter(conn);
	}
	return TRUE;
}

CPasvFilter::CPasvFilter(CBaseFilter *next)
	:CBaseFilter(next)
{

}

BOOL CPasvFilter::filter(FTP_Connection_s *conn)
{
	BOOL	ret = CBaseFilter::filter(conn);
	if(ret == TRUE)
	{
		return ret;
	}
	if(strncmp(conn->m_command,"list",conn->m_commandLen)==0
		|| strncmp(conn->m_command,"retr",conn->m_commandLen)==0
		|| strncmp(conn->m_command,"stor",conn->m_commandLen)==0)
	{
		if(conn->m_status != FTP_OPEN_DATA_CHANNEL)
		{
			char		errMsg[] = "502 please use pasv command\r\n";
			bufferevent_write(conn->m_bev,errMsg,strlen(errMsg));
			return FALSE;
		}
	}
	if(m_next)
	{
		return m_next->filter(conn);
	}
	return TRUE;
}