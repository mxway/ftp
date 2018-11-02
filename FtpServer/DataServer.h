/***************************************************
*
*	@版本:	1.0
*	@作者:	Mengxl
*	@时间:	2018-10-29 17:21:53
*	@文件:	DataServer.h
*	@描述： 用于对ftp数据连接通道进行处理
*
****************************************************/
#ifndef DATA_SERVER_INCLUDE_H
#define DATA_SERVER_INCLUDE_H
#include "BaseThread.h"
#include "ftp.h"
#include <event2/event.h>
#include <event2/bufferevent.h>

class CDataServer:public CBaseThread
{
public:
	CDataServer(ULONG serverIP, int port, FTP_Connection_s *conn);
	~CDataServer();
	virtual		int		RunThread();

	int		StopThread();

	void		list(const CStringUtil &fileName);

	void		onAccept(int fd, short events);

	void		onRead(evutil_socket_t fd, short events);

	void		retrFile(const CStringUtil &fileName);

	void		storeFile(const CStringUtil &fileName);
private:
	evutil_socket_t	initTcpServer();
private:
	event_base			*m_base;
	evutil_socket_t		m_serverFd;
	ULONG				m_serverIP;
	int					m_port;
	FTP_Connection_s	*m_curConn;
	struct event		*m_clientEvent;
	HANDLE				m_fileHandle;
};


#endif