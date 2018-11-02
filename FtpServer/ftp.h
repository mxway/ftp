#ifndef FTP_DATA_INCLUDE_H
#define FTP_DATA_INCLUDE_H

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <map>
#include "StringUtil.h"

enum FTP_Status
{
	FTP_CONNECTED,
	FTP_NEED_PASSWORD,
	FTP_LOGIN,
	FTP_OPEN_DATA_CHANNEL
};

class CDataServer;

typedef struct
{
	int				m_fd;
	bufferevent		*m_bev;
	FTP_Status		m_status;//ftp连接当前状态
	CStringUtil			m_userName;//当前登录的用户名
	CStringUtil			m_rootDir;//当前用户根目录。
	char			*m_recvBuf;//从客户端已收到的数据
	UINT32			m_recvLen;//当前recvBuf已收到字节数。
	UINT32			m_recvCapacity;//m_recvBuf最大可存字节数
	char			*m_command;//当前正在处理的command命令
	UINT32			m_commandLen;//命令字符长度
	char			*m_param;//当前命令的参数值
	UINT32			m_paramLen;//参数字符长度
	CStringUtil		m_workDir;//当前工作目录
	CDataServer		*m_dataServer;//
}FTP_Connection_s;

class CFtpCommand
{
public:
	virtual void processCommand(FTP_Connection_s *conn) = 0;
};

#endif