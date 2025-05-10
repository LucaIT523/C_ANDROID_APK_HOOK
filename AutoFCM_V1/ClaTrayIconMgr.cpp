#include "pch.h"
#include "AutoFCMDlg.h"
#include "ClaTrayIconMgr.h"
#include "resource.h"

BEGIN_MESSAGE_MAP(ClaTrayIconMgr, CWnd)
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnTrayNotification1)
	ON_COMMAND(ID_TRAY_OPEN, &ClaTrayIconMgr::OnTrayOpen)
	ON_COMMAND(ID_TRAY_EXIT, &ClaTrayIconMgr::OnTrayExit)
END_MESSAGE_MAP()

ClaTrayIconMgr::ClaTrayIconMgr()
{
	memset(&m_tnd, 0, sizeof(NOTIFYICONDATA));
}

ClaTrayIconMgr::~ClaTrayIconMgr()
{
	Shell_NotifyIconW(NIM_DELETE, &m_tnd);
}

BOOL ClaTrayIconMgr::Create(CWnd* pParent, UINT uCallbackMessage, LPCWSTR szToolTip, HICON icon, UINT uID)
{
	// Ensure that the common control DLL is loaded.
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icc);

	// Create the tray icon
	m_tnd.cbSize = sizeof(NOTIFYICONDATA);
	m_tnd.hWnd = pParent->GetSafeHwnd();
	m_tnd.uID = uID;
	m_tnd.hIcon = icon;
	m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_tnd.uCallbackMessage = uCallbackMessage;
	wcscpy_s(m_tnd.szTip, 64, szToolTip);

	_pParent = pParent;
	return Shell_NotifyIcon(NIM_ADD, &m_tnd);
}

BOOL ClaTrayIconMgr::SetTooltipText(LPCTSTR szToolTip)
{
	if (!szToolTip)
		return FALSE;

	m_tnd.uFlags = NIF_TIP;
	wcscpy_s(m_tnd.szTip, 64, szToolTip);

	return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL ClaTrayIconMgr::SetIcon(HICON hIcon)
{
	if (!hIcon)
		return FALSE;

	m_tnd.uFlags = NIF_ICON;
	m_tnd.hIcon = hIcon;

	return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

LRESULT ClaTrayIconMgr::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	return OnTrayNotification(wParam, lParam);
}
LRESULT ClaTrayIconMgr::OnTrayNotification1(WPARAM wParam, LPARAM lParam)
{
	UINT uID;
	UINT uMsg;

	uID = (UINT)wParam;
	uMsg = (UINT)lParam;

	if (uID != 1)
		return 0;

	CPoint pt;
	switch (uMsg)
	{
	case WM_LBUTTONDBLCLK:
		// Set menu to foreground
		_pParent->SetForegroundWindow();
		_pParent->ShowWindow(SW_SHOW);

		break;
	case WM_RBUTTONDOWN:
	case WM_CONTEXTMENU:
		// Handle right-clicks here
		GetCursorPos(&pt);
		// Create a menu
		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING, ID_TRAY_OPEN, _T("Open"));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_TRAY_EXIT, _T("Exit"));

		_pParent->SetForegroundWindow();

		// TrackPopupMenu blocks the app until TrackPopupMenu returns
		UINT clicked = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, _pParent);

		// Send message to the window to handle the command
		if (clicked)
			_pParent->SendMessage(WM_COMMAND, clicked);

		break;
	}

	return 0;
}

// Implement these functions
void ClaTrayIconMgr::OnTrayOpen()
{
	_pParent->SetForegroundWindow();
	_pParent->ShowWindow(SW_SHOW);
}

void ClaTrayIconMgr::OnTrayExit()
{
	// Handle the 'Exit' menu item click here
	PostQuitMessage(0);
}

void ClaTrayIconMgr::ShowBalloonTip(LPCTSTR szText, LPCTSTR szTitle, DWORD dwIcon, UINT uTimeout)
{
	_pParent->ShowWindow(SW_SHOW);

	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = NULL;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
//	nid.dwInfoFlags = dwIcon;
	wcscpy_s(nid.szInfo, szText);
	wcscpy_s(nid.szInfoTitle, szTitle);

	BOOL bSts = Shell_NotifyIcon(NIM_MODIFY, &nid);

// 	// Ensure that the version is set to Windows 2000 or later
// 	NOTIFYICONDATA nid = { 0 };
// 	nid.cbSize = sizeof(NOTIFYICONDATA);
// 	nid.hWnd = _pParent->GetSafeHwnd(); // Handle to the window that receives notification messages
// 	nid.uFlags = NIF_INFO;
// 	nid.uTimeout = uTimeout; // Timeout for the balloon tip
// 	nid.dwInfoFlags = dwIcon; // Icon to be displayed in the balloon tip
// 
// 	// Copy the title and text to the structure
// 	_tcscpy_s(nid.szInfoTitle, sizeof(nid.szInfoTitle) / sizeof(TCHAR), szTitle);
// 	_tcscpy_s(nid.szInfo, sizeof(nid.szInfo) / sizeof(TCHAR), szText);
// 
// 	// Show the balloon tip
// 	Shell_NotifyIcon(NIM_MODIFY, &nid);
}
