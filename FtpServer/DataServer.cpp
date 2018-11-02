#include "stdafx.h"
#include "DataServer.h"
#include <event2/thread.h>

void	onAcceptCallback(int fd, short events, void *arg)
{
	CDataServer	*server = static_cast<CDataServer*>(arg);
	if(server==NULL)
	{
		return;
	}
	server->onAccept(fd,events);
}

void	onReadCallback(evutil_socket_t fd, short events, void *arg)
{
	CDataServer	*server = static_cast<CDataServer*>(arg);
	if (server == NULL)
	{
		return;
	}
	server->onRead(fd,events);
}

CDataServer::CDataServer(ULONG serverIP, int port, FTP_Connection_s *conn)
{
	m_serverIP = serverIP;
	m_port = port;
	m_curConn = conn;
	m_clientEvent = NULL;
	m_fileHandle = INVALID_HANDLE_VALUE;
}

CDataServer::~CDataServer()
{
	
}

int	CDataServer::RunThread()
{
	evthread_use_windows_threads();
	m_base = event_base_new();
	m_serverFd = this->initTcpServer();
	if (m_serverFd == -1)
	{
		return 1;
	}
	struct	event *ev_listen = event_new(m_base, m_serverFd,
		EV_READ | EV_PERSIST, onAcceptCallback, this);
	m_curConn->m_status = FTP_OPEN_DATA_CHANNEL;
	ULONG		localIP = ntohl(m_serverIP);
	event_add(ev_listen, NULL);
	CStringUtil		pasvMsg;
	pasvMsg.Format(TEXT("227 enter pasv mode (%d,%d,%d,%d,%d,%d)\r\n"), 
		(localIP&0xff000000)>>24,
		(localIP&0xff0000)>>16,
		(localIP&0xff00)>>8,
		(localIP&0xff),
		m_port/256,
		m_port%256);
	bufferevent_write(m_curConn->m_bev, pasvMsg.GetString(), pasvMsg.GetLength());
	event_base_dispatch(m_base);
	event_base_free(m_base);
	return 0;
}

int CDataServer::StopThread()
{
	m_curConn->m_status = FTP_LOGIN;
	event_base_loopbreak(m_base);
	return CBaseThread::StopThread();
}

evutil_socket_t	CDataServer::initTcpServer()
{
	evutil_socket_t	fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		return -1;
	}
	evutil_make_listen_socket_reuseable(fd);

	struct  sockaddr_in		sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = m_serverIP;
	sin.sin_port = htons(m_port);

	if (::bind(fd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		goto ERR;
	}
	if (::listen(fd, 10) < 0)
	{
		goto ERR;
	}
	evutil_make_socket_nonblocking(fd);
	return fd;
ERR:
	evutil_closesocket(fd);
	return -1;
}

void CDataServer::list(const CStringUtil &fileName)
{
	CStringUtil	filter = fileName;
	filter += TEXT("/*.*");
	WIN32_FIND_DATA		findData = { 0 };
	char	tmpMsg[] = "150 open data channel for transffering data\r\n";
	bufferevent_write(m_curConn->m_bev, tmpMsg, strlen(tmpMsg));
	int		tmpCount = 500;
	while (m_clientEvent == NULL && tmpCount--)
	{
		Sleep(10);
	}
	if (m_clientEvent == NULL)
	{
		char	errMsg[] = "440 can not open data\r\n";
		bufferevent_write(m_curConn->m_bev, errMsg, strlen(errMsg));
		return;
	}
	HANDLE	fileHandle = ::FindFirstFile(filter.GetString(), &findData);
	BOOL		ret = TRUE;
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		char	msg[] = "530 invalid file path\r\n";
		bufferevent_write(m_curConn->m_bev, msg, strlen(msg));
		return;
	}
	CStringUtil	fileInfo;
	evutil_socket_t	sockfd = event_get_fd(m_clientEvent);
	while (ret)
	{
		if (findData.cFileName[0] == TEXT('.'))
		{
			ret = ::FindNextFile(fileHandle, &findData);
			continue;
		}
		
		fileInfo.Format(TEXT("%crwxr-xr-x 1 ftp ftp%15d Jul 01 23:02 %s\r\n"),
			findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? 'd' : '-',
			findData.nFileSizeLow, findData.cFileName);
		if(send(sockfd,fileInfo.GetString(),fileInfo.GetLength(),0)<=0)
		{
			goto CLOSE;
		}
		ret = ::FindNextFile(fileHandle, &findData);
	}
	::FindClose(fileHandle);
	char		okMsg[] = "226 list complete\r\n";
	bufferevent_write(m_curConn->m_bev, okMsg, strlen(okMsg));
CLOSE:
	event_del(m_clientEvent);
	evutil_closesocket(sockfd);
	event_free(m_clientEvent);
}

void CDataServer::onAccept(int fd, short events)
{
	evutil_socket_t	sockfd;
	struct sockaddr_in		client;
	socklen_t	len = sizeof(client);

	sockfd = ::accept(fd, (struct sockaddr*)&client, &len);
	evutil_make_socket_nonblocking(sockfd);
	m_clientEvent= event_new(m_base,sockfd,EV_READ|EV_PERSIST,
		onReadCallback,this);
	event_add(m_clientEvent,NULL);
}

void CDataServer::onRead(evutil_socket_t fd, short events)
{
	int		tmpCount = 500;
	char	*buf = NULL;
	while (m_fileHandle == INVALID_HANDLE_VALUE && tmpCount--)
	{
		Sleep(10);
	}
	if(m_fileHandle==INVALID_HANDLE_VALUE)
	{
		char		msg[] = "444 can not open file\r\n";
		bufferevent_write(m_curConn->m_bev,msg,strlen(msg));
		goto CLOSE;
	}
	buf = (char*)malloc(1024*1024);
	if(buf == NULL)
	{
		char		errMsg[] = "430 no enough memory\r\n";
		bufferevent_write(m_curConn->m_bev,errMsg,strlen(errMsg));
		goto CLOSE;
	}
	int		bytes = recv(fd, buf, 1024 * 1024, 0);
	while(1)
	{
		if(bytes == 0)
		{
			//文件传输完成，通道被关闭
			char	okMsg[] = "200 ok\r\n";
			bufferevent_write(m_curConn->m_bev,okMsg,strlen(okMsg));
			goto CLOSE;
		}
		if(bytes < 0)
		{
			/*char		errMsg[] ="533 socket error\r\n";
			bufferevent_write(m_curConn->m_bev,errMsg,strlen(errMsg));
			goto CLOSE;*/
		}
		DWORD		writeBytes = 0;
		::WriteFile(m_fileHandle,buf,bytes,&writeBytes,NULL);
		bytes = recv(fd,buf,804608,0);
	}
	free(buf);
	return;
CLOSE:
	free(buf);
	if(m_fileHandle!=INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_fileHandle);
	}
	event_del(m_clientEvent);
	evutil_closesocket(fd);
	event_free(m_clientEvent);
}

void CDataServer::retrFile(const CStringUtil &fileName)
{
	char	tmpMsg[] = "150 open data channel for transffering data\r\n";
	bufferevent_write(m_curConn->m_bev, tmpMsg, strlen(tmpMsg));
	int		tmpCount = 500;
	while (m_clientEvent == NULL && tmpCount--)
	{
		Sleep(10);
	}
	if (m_clientEvent == NULL)
	{
		char	errMsg[] = "440 can not open data\r\n";
		bufferevent_write(m_curConn->m_bev, errMsg, strlen(errMsg));
		return;
	}
	HANDLE		fileHandle = ::CreateFile(fileName.GetString(),
		GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(fileHandle==INVALID_HANDLE_VALUE)
	{
		char	msg[] = "530 can not open file for transfer\r\n";
		bufferevent_write(m_curConn->m_bev,msg,strlen(msg));
		return;
	}
	char	*buf = (char*)malloc(1024*1024);
	if(buf==NULL)
	{
		::CloseHandle(fileHandle);
		char	msg[] = "440 have no enough memeory\r\n";
		bufferevent_write(m_curConn->m_bev,msg,strlen(msg));
		return;
	}
	DWORD		fileSize = ::GetFileSize(fileHandle,NULL);
	DWORD		bytes = 0;
	evutil_socket_t	sockfd = event_get_fd(m_clientEvent);
	while(bytes<fileSize)
	{
		DWORD	readBytes = 0;
		::ReadFile(fileHandle,buf,1024*1024,&readBytes,NULL);
		if(send(sockfd,buf,readBytes,0)<=0)
		{
			char	errMsg[] = "330 retr file uncomplete\r\n";
			bufferevent_write(m_curConn->m_bev,errMsg,strlen(errMsg));
			goto END;
		}
		bytes += readBytes;
	}
	char	okMsg[] = "220 retr file completed\r\n";
	bufferevent_write(m_curConn->m_bev,okMsg,strlen(okMsg));
END:
	::CloseHandle(fileHandle);
	free(buf);
	event_del(m_clientEvent);
	evutil_closesocket(sockfd);
	event_free(m_clientEvent);
}

#if 0
void CDataServer::onClose(bufferevent *bev, short event)
{
	bufferevent_free(bev);
}
#endif

void CDataServer::storeFile(const CStringUtil &fileName)
{
	char	tmpMsg[] = "150 open data channel for transffering data\r\n";
	bufferevent_write(m_curConn->m_bev, tmpMsg, strlen(tmpMsg));
	int		tmpCount = 500;
	m_fileHandle = ::CreateFile(fileName.GetString(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,
		NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
}
