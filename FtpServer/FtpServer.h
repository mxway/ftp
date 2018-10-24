#ifndef FTP_SERVER_INCLUDE_H
#define FTP_SERVER_INCLUDE_H
#include "ftp.h"
#include "BaseThread.h"
#include <vector>

class CFtpServer:public CBaseThread
{
public:
	CFtpServer();
	~CFtpServer();
	void	onAccept(evconnlistener *listener,evutil_socket_t fd,
		struct sockaddr *sock,int socklen);
	void	onRead(bufferevent *bev);
	void	onClose(bufferevent *bev,short events);
	int		RunThread();
	int		StopThread();
private:
	void	initServer();
	void	registerCallbacks();
	void	processFtpCommand(FTP_Connection_s *conn);
	void	closeConnections();
	void	unRegisterCallbacks();
private:
	event_base	*m_base;
	std::map<evutil_socket_t,FTP_Connection_s*>	m_connections;
	std::map<std::string,CFtpCommand*>	m_callbacks;
	std::vector<CFtpCommand*>			m_callbackArray;
};

#endif