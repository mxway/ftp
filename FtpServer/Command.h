/***************************************************
*
*	@Version:	1.0
*	@Author:	Mengxl
*	@Date:		2018-10-23 21:47:19
*	@File:		Command.h
*
****************************************************/
#pragma once
#include "ftp.h"
#include "StringUtil.h"

CStringUtil	GetRealPath(LPCTSTR prmPath);

CStringUtil	GetParentPath(LPCTSTR prmPath);

class CFtpLoginCommand :public CFtpCommand
{
public:
	void		processCommand(FTP_Connection_s *conn);
};

class CPwdCommand :public CFtpCommand
{
public:
	void		processCommand(FTP_Connection_s *conn);
};

class CCwdCommand :public CFtpCommand
{
public:
	void		processCommand(FTP_Connection_s *conn);
};

class CCDupCommand:public CFtpCommand
{
public:
	void		processCommand(FTP_Connection_s *conn);
};

class CNoopCommand:public CFtpCommand
{
public:
	void		processCommand(FTP_Connection_s *conn);
};

class CQuitCommand :public CFtpCommand
{
public:
	void processCommand(FTP_Connection_s *conn);
};