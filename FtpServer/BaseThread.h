
#ifndef BASE_THREAD_INCLUDE_H
#define BASE_THREAD_INCLUDE_H

#include <windows.h>
#include "StringUtil.h"

#define WM_RUNTHREAD		WM_USER+0x100
#define WM_COMPLETETHREAD	WM_USER+0x101

unsigned int __stdcall ThreadFunc(LPVOID lParam);
typedef struct
{
	CStringUtil  m_threadName;//线程名称
	HWND    m_notifyWnd;//线程启动，停止的消息通知哪个窗口
	HANDLE	m_threadHandle;//线程的句柄
	UINT32	m_threadId;//线程ID
}Thread_s;

class CBaseThread
{
public:
	CBaseThread() :m_thread(NULL), m_runningFlag(1) {}
	~CBaseThread() { delete m_thread; }
	/**
	*
	*  创建线程
	*  
	*
	**/
	virtual int  CreateThread(CStringUtil name = TEXT(""));

	/**
	*
	* 线程启动函数，如果执行成功返回1，否则返回0
	*
	**/
	virtual int StartThread();

	/**
	*
	* 线程运行函数
	*
	**/
	virtual	int RunThread() = 0;

	/***
	*
	* 线程停止函数，如果执行成功返回1，否则返回0
	*
	*
	**/
	virtual int StopThread();

	/**
	*
	* 设置线程消息通知的窗口
	*
	*/
	void   SetNotifyWnd(HWND hwnd);

	virtual void CompleteThread() {}

	void   SetRunningFlage(BOOL prmFlage) { m_runningFlag = prmFlage; }

	Thread_s	*GetThreadStruct() { return m_thread; }

	int		WaitThread();
		
protected:
	/**
	*
	* 该函数用于自定义停止线程的方法，建议不要使用TerminateThread
	*	和ExitThread来实现，而是在循环变量加入一个变量判断是否结束进程函数
	*	建议在子类Run方法中，使用m_runningFlag作为循环的判断条件之一，当m_runningFlag为1时
	*	线程函数可一直运行，当m_runningFlag为0时，线程运行结束
	*
	**/
	virtual int Stop() { m_runningFlag = 0; return 0; };
	Thread_s  *m_thread;
	UINT32	 m_runningFlag;
};

#endif