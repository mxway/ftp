/***************************************************
*
*	@版本:	1.0
*	@作者:	Mengxl
*	@时间:	2018-11-02 10:55:17
*	@文件:	Filter.h
*	@描述：
*
****************************************************/

#ifndef FILTER_INCLUDE_H
#define FILTER_INCLUDE_H
#include "ftp.h"

class CBaseFilter
{
public:
	CBaseFilter(CBaseFilter *filter = NULL);
	virtual	~CBaseFilter(){}
	virtual BOOL	filter(FTP_Connection_s *conn);
	void			setNextFilter(CBaseFilter *next){m_next = next;}
protected:
	CBaseFilter		*m_next;
};

class CLogStatusFilter:public CBaseFilter
{
public:
	CLogStatusFilter(CBaseFilter *next=NULL);
	virtual	BOOL	filter(FTP_Connection_s *conn);
};

class CPasvFilter:public CBaseFilter
{
public:
	CPasvFilter(CBaseFilter *next=NULL);
	virtual BOOL  filter(FTP_Connection_s *conn);
};

#endif