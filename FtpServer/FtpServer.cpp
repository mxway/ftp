#include "stdafx.h"
#include "FtpServer.h"
#include <WinSock.h>
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include "Command.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"../lib/libevent_core.lib")

void listener_cb(evconnlistener *listener,evutil_socket_t fd,
	struct sockaddr *sock,int socklen,void *arg)
{
	CFtpServer	*serverObj = static_cast<CFtpServer*>(arg);
	if(serverObj==NULL)
	{
		return;
	}
	serverObj->onAccept(listener,fd,sock,socklen);
}

void socket_read_cb(bufferevent *bev,void *arg)
{
	CFtpServer	*serverObj = static_cast<CFtpServer*>(arg);
	if(serverObj==NULL)
	{
		return;
	}
	serverObj->onRead(bev);
}

void socket_close_cb(bufferevent *bev,short events,void *arg)
{
	CFtpServer	*serverObj = static_cast<CFtpServer*>(arg);
	if(serverObj==NULL)
	{
		return;
	}
	serverObj->onClose(bev,events);
}

CFtpServer::CFtpServer()
	:m_base(NULL)
{
	WSAData	wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	this->registerCallbacks();
	this->initServer();
}

CFtpServer::~CFtpServer()
{
	WSACleanup();
	this->unRegisterCallbacks();
}

void CFtpServer::onAccept(evconnlistener *listener,evutil_socket_t fd, struct sockaddr *sock,int socklen)
{
	struct event_base *base = evconnlistener_get_base(listener);
	if(base != m_base)
	{
		return;
	}
	char	msg[]="220 the ftp server is written by mengxl\r\n";
	bufferevent *bev = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev,socket_read_cb,NULL,socket_close_cb,this);
	struct timeval tv = { 10, 0 };
	bufferevent_set_timeouts(bev, &tv, NULL);
	FTP_Connection_s	*conn = new FTP_Connection_s;
	conn->m_fd = fd;
	conn->m_bev = bev;
	conn->m_status = FTP_CONNECTED;
	conn->m_recvBuf = (char*)malloc(130);
	conn->m_recvLen = 0;
	conn->m_recvCapacity = 128;
	conn->m_workDir = "/";
	conn->m_commandLen = 0;
	conn->m_paramLen = 0;
	m_connections.insert(std::make_pair(fd,conn));
	bufferevent_write(bev,msg,strlen(msg));
	bufferevent_enable(bev,EV_READ|EV_PERSIST);
}

void CFtpServer::onRead(bufferevent *bev)
{
	char		buf[1024] = {0};
	int			bytes = 0;
	evutil_socket_t	sockfd = bufferevent_getfd(bev);
	std::map<evutil_socket_t,FTP_Connection_s*>::iterator	tmpItr = m_connections.find(sockfd);
	if(tmpItr == m_connections.end())
	{
		return;
	}
	FTP_Connection_s	*conn = tmpItr->second;
	while( (bytes=bufferevent_read(bev,buf,1024)) >0)
	{
		if(conn->m_recvLen+bytes > conn->m_recvCapacity)
		{
			int		reallocSize = bytes>conn->m_recvCapacity?bytes:conn->m_recvCapacity;
			realloc(conn->m_recvBuf,reallocSize*2+2);
			conn->m_recvCapacity = reallocSize*2;
		}
		memcpy(conn->m_recvBuf + conn->m_recvLen,buf,bytes);
		conn->m_recvLen += bytes;
	}
	if(conn->m_recvLen>2 &&
		conn->m_recvBuf[conn->m_recvLen-2]=='\r'
		&& conn->m_recvBuf[conn->m_recvLen-1]=='\n')
	{
		//接收到完整的ftp请求命令行。
		this->processFtpCommand(conn);
	}
}

void CFtpServer::onClose(bufferevent *bev,short events)
{
	evutil_socket_t	sockfd = bufferevent_getfd(bev);
	std::map<evutil_socket_t,FTP_Connection_s*>::iterator tmpItr = m_connections.find(sockfd);
	if(tmpItr==m_connections.end())
	{
		return;
	}
	FTP_Connection_s *conn = tmpItr->second;
	free(conn->m_recvBuf);
	m_connections.erase(tmpItr);
	delete conn;
	bufferevent_free(bev);
}

int CFtpServer::RunThread()
{
	event_base_dispatch(m_base);
	return 0;
}

int CFtpServer::StopThread()
{
	event_base_loopbreak(m_base);
	this->closeConnections();
	return CBaseThread::StopThread();
}

void CFtpServer::initServer()
{
	evthread_use_windows_threads();
	struct sockaddr_in	sin;
	memset(&sin,0,sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.s_addr = ADDR_ANY;
	m_base = event_base_new();
	evconnlistener *listener = evconnlistener_new_bind(m_base,
		listener_cb,this,
		LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,
		10,(struct sockaddr*)&sin,
		sizeof(struct sockaddr_in));
}

void CFtpServer::registerCallbacks()
{
	CFtpLoginCommand *logCommand = new CFtpLoginCommand;
	CPwdCommand		 *pwdComand = new CPwdCommand;
	CQuitCommand	 *quitCommand = new CQuitCommand;

	m_callbackArray.push_back(logCommand);
	m_callbackArray.push_back(pwdComand);
	m_callbackArray.push_back(quitCommand);

	m_callbacks.insert(std::make_pair("user",logCommand));
	m_callbacks.insert(std::make_pair("pass",logCommand));
	m_callbacks.insert(std::make_pair("pwd",pwdComand));
	m_callbacks.insert(std::make_pair("quit",quitCommand));
}

void CFtpServer::processFtpCommand(FTP_Connection_s *conn)
{
	char		command[10] = {0};
	char	*p = conn->m_recvBuf;
	//一次tcp发送的数据可能包含多个命令,使用while循环处理
	while(p < conn->m_recvBuf+conn->m_recvLen)
	{
		while(isspace(*p))p++;
		conn->m_command = p;
		while(!isspace(*p))
		{
			if(*p>='A' && *p<='Z')
			{
				*p = *p+32;
			}
			p++;
		}
		conn->m_commandLen = p-conn->m_command;
		while(*p==' ' || *p=='\t')p++;
		if(*p=='\r' && *(p+1)=='\n')
		{
			conn->m_param = NULL;
			conn->m_paramLen = 0;
		}
		else
		{
			conn->m_param = p;
			while(*p!='\r' && *(p+1)!='\n')p++;
			conn->m_paramLen = p - conn->m_param;
		}
		memcpy(command,conn->m_command,conn->m_commandLen);
		std::map<std::string,CFtpCommand*>::iterator tmpItr = m_callbacks.find(command);
		if(tmpItr!=m_callbacks.end())
		{
			tmpItr->second->processCommand(conn);
		}
		else
		{
			std::string		str = "550 unknown command\r\n";
			bufferevent_write(conn->m_bev,str.c_str(),str.length());
		}
		if(strncmp(command,"quit",4)==0)
		{
			this->onClose(conn->m_bev,0);
			return;
		}
		p = p+2;
	}
	conn->m_recvLen = 0;
}

void CFtpServer::closeConnections()
{
	std::map<evutil_socket_t,FTP_Connection_s*>::iterator tmpItr = m_connections.begin();
	while(tmpItr != m_connections.end())
	{
		FTP_Connection_s *conn = tmpItr->second;
		bufferevent_free(conn->m_bev);
		free(conn->m_recvBuf);
		delete conn;
		tmpItr++;
	}
	m_connections.clear();
}

void CFtpServer::unRegisterCallbacks()
{
	std::vector<CFtpCommand*>::iterator tmpItr = m_callbackArray.begin();
	while (tmpItr != m_callbackArray.end())
	{
		delete (*tmpItr);
		tmpItr++;
	}
	m_callbackArray.clear();
	m_callbacks.clear();
}