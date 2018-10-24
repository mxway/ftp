#include <process.h>
#include "stdafx.h"
#include "BaseThread.h"

unsigned int __stdcall ThreadFunc(LPVOID lParam)
{
	UINT32 result = 0;
	CBaseThread	*thread = (CBaseThread*)lParam;
	Thread_s *tmpThreadStruct = thread->GetThreadStruct();
	::PostMessage(tmpThreadStruct->m_notifyWnd, WM_RUNTHREAD, 0,(LPARAM)thread);
#ifndef _DEBUG
	try
	{
#endif
		result = thread->RunThread();
#ifndef _DEBUG
	}
	catch (...)
	{
		thread->CompleteThread();
		::SendMessage(tmpThreadStruct->m_notifyWnd, WM_COMPLETETHREAD, 0, (LPARAM)thread);
		::CloseHandle(tmpThreadStruct->m_threadHandle);
		tmpThreadStruct->m_threadHandle = INVALID_HANDLE_VALUE;
		return 0;
		//return -1;
	}
#endif
	thread->CompleteThread();
	thread->SetRunningFlage(FALSE);
	::PostMessage(tmpThreadStruct->m_notifyWnd, WM_COMPLETETHREAD, 0,(LPARAM)thread);
	::CloseHandle(tmpThreadStruct->m_threadHandle);
	tmpThreadStruct->m_threadHandle = INVALID_HANDLE_VALUE;
	return result;
}

int CBaseThread::CreateThread(CStringUtil name /* = "" */)
{
	if (m_thread && 
		m_thread->m_threadHandle!=INVALID_HANDLE_VALUE &&
		m_thread->m_threadHandle!=NULL)
	{
		this->StopThread();
		::WaitForSingleObject(m_thread->m_threadHandle, INFINITE);
		::CloseHandle(m_thread->m_threadHandle);
	}
	else if(m_thread==NULL)
	{
		m_thread = new Thread_s;
		if (m_thread == NULL)
		{
			return 1;
		}
	}

	this->m_runningFlag = 1;
	m_thread->m_threadHandle = (HANDLE) ::_beginthreadex(NULL, NULL, ThreadFunc, this, CREATE_SUSPENDED, &m_thread->m_threadId);
	m_thread->m_threadName = name;

	return 0;
}

//等待线程执行完成
int CBaseThread::WaitThread()
{
	//5s线程还没有执行完成，就强制退出程序
	int		tmpCount = 500;
	while(tmpCount--)
	{
		if (m_thread == NULL || m_thread->m_threadHandle == INVALID_HANDLE_VALUE)
		{
			break;
		}
		Sleep(10);
	}
	return 1;
}

int CBaseThread::StartThread()
{
	if (m_thread && m_thread->m_threadHandle)
	{
		::ResumeThread(m_thread->m_threadHandle);
		return 0;
	}
	return 1;
}

void CBaseThread::SetNotifyWnd(HWND hwnd)
{
	if (m_thread)
	{
		m_thread->m_notifyWnd = hwnd;
	}
}

int CBaseThread::StopThread()
{
	if (m_thread == NULL || m_thread->m_threadHandle == NULL)
	{
		return 1;
	}
	this->Stop();
	return 0;
}