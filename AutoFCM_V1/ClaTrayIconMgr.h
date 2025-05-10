#pragma once

#include <afxwin.h>

#define WM_TRAY_NOTIFICATION	(WM_USER+0x2001)

class ClaTrayIconMgr : public CWnd
{
public:
public:
	ClaTrayIconMgr();
	virtual ~ClaTrayIconMgr();

	BOOL Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szToolTip, HICON icon, UINT uID);
	BOOL SetTooltipText(LPCTSTR szToolTip);
	BOOL SetIcon(HICON hIcon);
	void ShowBalloonTip(LPCTSTR szText, LPCTSTR szTitle, DWORD dwIcon, UINT uTimeout);

public:
	LRESULT OnTrayNotification1(WPARAM wParam, LPARAM lParam);

protected:
	NOTIFYICONDATAW m_tnd;

	// Message map functions
	afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	void OnTrayOpen();
	void OnTrayExit();

	DECLARE_MESSAGE_MAP()

	CWnd* _pParent;
};

