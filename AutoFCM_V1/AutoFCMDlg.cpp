
// AutoFCMDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "AutoFCM_V1.h"
#include "AutoFCMDlg.h"
#include "afxdialogex.h"

#include "ClaRegMgr.h"
#include "ClaPathMgr.h"
#include "base64enc.h"
#include <afxdialogex.h>
#include ".\rapidxml-1.13\rapidxml.hpp"
#include ".\rapidxml-1.13\rapidxml_print.hpp"
#include <Shellapi.h>
#include <atlstr.h> // Include MFC's CString header
#include <fstream>
#include <vector>
#include <filesystem> // For directory iteration and path parsing
#include <shlobj.h> // Include Windows Shell header

using namespace rapidxml;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_ADD_LOG (WM_USER+0x1001)
#define WM_ADD_RESULT (WM_USER+0x1002)
#define WM_FINISHED (WM_USER+0x1003)

HANDLE lv_hThreadConvert = NULL;
HANDLE lv_hMutex = NULL;

unsigned long TF_CONVERT_ALL(void* p_pParam) {
	CAutoFCMDlg* pThis = (CAutoFCMDlg*)p_pParam;
	pThis->convert_all();
	lv_hThreadConvert = NULL;
	return 0;
}

unsigned long TF_CONVERT(void* p_pParam) {
	ST_THREAD_PARAM* pThis = (ST_THREAD_PARAM*)p_pParam;
	pThis->m_pThis->convert(pThis->m_nIndex);
	delete p_pParam;
	return 0;
}

// CAutoFCMDlg dialog

CAutoFCMDlg::CAutoFCMDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_AUTOFCM_V1_DIALOG, pParent)
	, m_strSrc(_T(""))
	, m_strOut(_T(""))
	, m_strServerURL(_T("http://"))
	, m_strLaunchButton(_T(""))
	, m_strLaunchURL(_T("http://"))
	, _nStatus(0)
	, m_strStatus(_T(""))
	, m_bFlagTerminate(false)
	, m_strTextSrc(_T("AN1.COM"))
	, m_strTextDst(_T("my.com"))
	, m_strBearer(_T("8|hgPRXqmAPyRjqtEIGhDkoVV949WUAD1X1iNPuMpa2c7458dd"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoFCMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_SRC, m_strSrc);
	DDX_Text(pDX, IDC_EDT_OUT, m_strOut);
	DDX_Text(pDX, IDC_EDT_SERVER_URL, m_strServerURL);
	DDX_Text(pDX, IDC_EDT_LAUNCH_BUTTON, m_strLaunchButton);
	DDX_Text(pDX, IDC_EDT_LAUNCH_URL, m_strLaunchURL);
	DDX_Control(pDX, IDC_LST_PROCESS, m_lstProcess);
	DDX_Text(pDX, IDC_STC_STATUS, m_strStatus);
	DDX_Text(pDX, IDC_EDT_SRC_TEXT, m_strTextSrc);
	DDX_Text(pDX, IDC_EDT_DST_TEXT, m_strTextDst);
	DDX_Text(pDX, IDC_EDT_BEARER, m_strBearer);
}

BEGIN_MESSAGE_MAP(CAutoFCMDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SRC, &CAutoFCMDlg::OnBnClickedBtnSrc)
	ON_BN_CLICKED(IDC_BTN_OUT, &CAutoFCMDlg::OnBnClickedBtnOut)
	ON_BN_CLICKED(IDOK, &CAutoFCMDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CAutoFCMDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDSTOP, &CAutoFCMDlg::OnBnClickedStop)
	ON_COMMAND(ID_TRAY_OPEN, &CAutoFCMDlg::OnTrayOpen)
	ON_COMMAND(ID_TRAY_EXIT, &CAutoFCMDlg::OnTrayExit)
	ON_BN_CLICKED(IDCLEAR, &CAutoFCMDlg::OnBnClickedClear)
END_MESSAGE_MAP()


// CAutoFCMDlg message handlers

BOOL CAutoFCMDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	_loadSetting();

	_initUI();
	_refreshUI();

	SetWindowTextW(L"AutoFCM v1.0.0.3");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAutoFCMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAutoFCMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CAutoFCMDlg::OnBnClickedBtnSrc()
{
	UpdateData(TRUE);

	CFolderPickerDialog dlg(m_strSrc, 0, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	m_strSrc = dlg.GetPathName();

	UpdateData(FALSE);

	_refreshUI();
}


void CAutoFCMDlg::OnBnClickedBtnOut()
{
	UpdateData(TRUE);

	CFolderPickerDialog dlg(m_strOut, 0, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	m_strOut = dlg.GetPathName();

	UpdateData(FALSE);
}

void CAutoFCMDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	if (_nStatus == 0) {
		_saveSetting();

		//.	not started
		if (_isCorrectDir(m_strSrc) == FALSE) {
			AfxMessageBox(L"Please select the source directory correctly."); return;
		}

		if (m_strOut.IsEmpty()) {
			if (AfxMessageBox(L"Do you want to set the output directory to source directory?", MB_OKCANCEL) == IDOK){
				m_strOut.Format(L"%s\\FCM Output", m_strSrc);
				UpdateData(FALSE);
			}
		}

		CreateDirectory(m_strOut, NULL);
		if (_isCorrectDir(m_strOut) == FALSE) {
			AfxMessageBox(L"Please select the output directory correctly."); return;
		}

		if (m_strServerURL.IsEmpty()) {
			AfxMessageBox(L"Please set the server url."); return;
		}

		if (m_strServerURL.Left(7).MakeUpper() != L"HTTP://" && m_strServerURL.Left(8).MakeUpper() != L"HTTPS://") {
			AfxMessageBox(L"URL must be begin with \"http://\"."); return;
		}

		_enumApkFiles(m_strSrc);

		if (_lstPaths.GetCount() == 0) {
			AfxMessageBox(L"No apk file was founded."); return;
		}

		CString strMutex; strMutex.Format(L"Mutex-%s", m_strSrc);
		strMutex.Replace(L":", L">");
		strMutex.Replace(L"\\", L"_");
		if ((lv_hMutex = OpenMutex(SYNCHRONIZE, FALSE, strMutex)) != NULL) {
			CloseHandle(lv_hMutex);
			AfxMessageBox(L"You have already started to convert this directory in other process.");
			return;
		}

		lv_hMutex = CreateMutexW(NULL, TRUE, strMutex);

		m_nSuccess = m_nFailed = 0;

		CString strCount; strCount.Format(L"AutoFCM - total : %d, success : %d, fail : %d",
			_lstPaths.GetCount(), m_nSuccess, m_nFailed
		);
		SetWindowTextW(strCount);

		_nStatus = 1;
		DWORD dwTID;
		lv_hThreadConvert = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TF_CONVERT_ALL, (LPVOID)this, 0, &dwTID);
		m_strStatus = L"Converting...";
		_refreshUI();
	}
	else if (_nStatus == 1){
		// started
		_nStatus = 2;
		m_strStatus = L"Pause...";
		_refreshUI();
	}
	else if (_nStatus == 2){
		// paused
		_nStatus = 1;
		m_strStatus = L"Converting...";
		_refreshUI();
	}

}

void CAutoFCMDlg::OnBnClickedStop()
{
	_nStatus = 3;
	m_strStatus = L"Stopping";
	UpdateData(FALSE);
	_refreshUI();
//	WaitForSingleObject(lv_hThreadConvert, INFINITE);
// 	m_strStatus = L"Stoped";
// 	_nStatus = 0;
// 	_refreshUI();
}

void CAutoFCMDlg::OnBnClickedCancel()
{
	_trayMgr.ShowBalloonTip(L"If you want to see the window, double click me!!!", L"Auto FCM", NIIF_INFO, 5000);
	ShowWindow(SW_HIDE);
	// TODO: Add your control notification handler code here
	//CDialog::OnCancel();
}

// Implement these functions
void CAutoFCMDlg::OnTrayOpen()
{
	SetForegroundWindow();
	ShowWindow(SW_SHOW);
}

void CAutoFCMDlg::OnTrayExit()
{
	UpdateData(TRUE);

	// Handle the 'Exit' menu item click here
	if (_nStatus != 0) {
		if (AfxMessageBox(L"It's converting now.\nDo you really want to close this program?", MB_OKCANCEL) != IDOK) {
			return;
		}
		m_bFlagTerminate = true;
		OnBnClickedStop();
	}else {
		if (AfxMessageBox(L"Do you want to close this program?", MB_OKCANCEL) != IDOK) {
			return;
		}
		_saveSetting();
		PostQuitMessage(0);
	}
}

void CAutoFCMDlg::_initUI()
{
	m_lstProcess.SetExtendedStyle(m_lstProcess.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_lstProcess.setHead(L"No;Path;Result;Status");

	m_lstProcess.autoFitWidth();

	_trayMgr.Create(this, WM_TRAY_NOTIFICATION, L"AutoFCM", m_hIcon, 1);
}

void CAutoFCMDlg::_refreshUI()
{
	if (_nStatus == 0) {
		GetDlgItem(IDC_EDT_SRC)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_OUT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_SRC)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_OUT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_SERVER_URL)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_LAUNCH_BUTTON)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_LAUNCH_URL)->EnableWindow(TRUE);
		GetDlgItem(IDSTOP)->EnableWindow(FALSE);
		GetDlgItem(IDOK)->EnableWindow(!m_strSrc.IsEmpty());
		GetDlgItem(IDOK)->SetWindowTextW(L"Start");
		GetDlgItem(IDCLEAR)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_SRC_TEXT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_DST_TEXT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDT_BEARER)->EnableWindow(TRUE);
	}
	else if (_nStatus == 1) {
		GetDlgItem(IDC_EDT_SRC)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_OUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SRC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_OUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_SERVER_URL)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_LAUNCH_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_LAUNCH_URL)->EnableWindow(FALSE);
		GetDlgItem(IDSTOP)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->SetWindowTextW(L"Pause");
		GetDlgItem(IDCLEAR)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_SRC_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_DST_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_BEARER)->EnableWindow(FALSE);
	}
	else if (_nStatus == 2) {
		GetDlgItem(IDC_EDT_SRC)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_OUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SRC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_OUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_SERVER_URL)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_LAUNCH_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_LAUNCH_URL)->EnableWindow(FALSE);
		GetDlgItem(IDSTOP)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->SetWindowTextW(L"Resume");
		GetDlgItem(IDCLEAR)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_SRC_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_DST_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_BEARER)->EnableWindow(FALSE);
	}
	else if (_nStatus == 3) {
		GetDlgItem(IDC_EDT_SRC)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_OUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SRC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_OUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_SERVER_URL)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_LAUNCH_BUTTON)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_LAUNCH_URL)->EnableWindow(FALSE);
		GetDlgItem(IDSTOP)->EnableWindow(FALSE);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDCLEAR)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_SRC_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_DST_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDT_BEARER)->EnableWindow(FALSE);
	}
	UpdateData(FALSE);
}

void CAutoFCMDlg::_enumApkFiles(const wchar_t* p_wszPath)
{
	CFileFind fileFind;
	CString strFind;

	_lstPaths.RemoveAll();

	strFind.Format(L"%s\\*.apk", p_wszPath);
	strFind.Replace(L"\\\\", L"\\");

	BOOL bSts = fileFind.FindFile(strFind);
	while (bSts)
	{
		bSts = fileFind.FindNextFileW();

		_lstPaths.Add(fileFind.GetFilePath());
	}

	m_lstProcess.DeleteAllItems();
	for (int i = 0; i < _lstPaths.GetCount(); i++) {
		CString strNo; strNo.Format(L"%d", i + 1);
		CString strPath = ClaPathMgr::GetFN(_lstPaths[i].GetBuffer());
		CString strStatus = L"";

		m_lstProcess.addRecord(strNo, strPath, L"", strStatus);
	}

	m_lstProcess.autoFitWidth();
}

//
//	for setting save / load
//

#define LD_SUBKEY_REG	L"Software\\AutoFCM"

void CAutoFCMDlg::_saveSetting()
{
	ClaRegMgr reg(HKEY_CURRENT_USER);

	reg.createKey(LD_SUBKEY_REG);
	//.	apk path
	reg.writeStringW(LD_SUBKEY_REG, L"apkpath", m_strSrc);
	//  out dir
	reg.writeStringW(LD_SUBKEY_REG, L"outdir", m_strOut);
	//  target url
	reg.writeStringW(LD_SUBKEY_REG, L"targeturl", m_strServerURL);
	//	button name
	reg.writeStringW(LD_SUBKEY_REG, L"button", m_strLaunchButton);
	//	launch url
	reg.writeStringW(LD_SUBKEY_REG, L"launch", m_strLaunchURL);
	//.	source text
	reg.writeStringW(LD_SUBKEY_REG, L"text_src", m_strTextSrc);
	//.	target text
	reg.writeStringW(LD_SUBKEY_REG, L"text_dst", m_strTextDst);
	//. bearer token
	reg.writeStringW(LD_SUBKEY_REG, L"bearer", m_strBearer);
}

void CAutoFCMDlg::_loadSetting()
{
	//
	ClaRegMgr reg(HKEY_CURRENT_USER);
	wchar_t wszTemp[MAX_PATH];

	memset(wszTemp, 0, sizeof(wszTemp));
	if (reg.readStringW(LD_SUBKEY_REG, L"apkpath", wszTemp) == 0) {
		m_strSrc = wszTemp;
	}

	//  out dir
	memset(wszTemp, 0, sizeof(wszTemp));
	if (reg.readStringW(LD_SUBKEY_REG, L"outdir", wszTemp) == 0) {
		m_strOut = wszTemp;
	}

	//  target url
	memset(wszTemp, 0, sizeof(wszTemp));
	if (reg.readStringW(LD_SUBKEY_REG, L"targeturl", wszTemp) == 0) {
		m_strServerURL = wszTemp;
	}

	//	button name
	memset(wszTemp, 0, sizeof(wszTemp));
	if (reg.readStringW(LD_SUBKEY_REG, L"button", wszTemp) == 0) {
		m_strLaunchButton = wszTemp;
	}

	//	launch url
	memset(wszTemp, 0, sizeof(wszTemp));
	if (reg.readStringW(LD_SUBKEY_REG, L"launch", wszTemp) == 0) {
		m_strLaunchURL= wszTemp;
	}

	memset(wszTemp, 0, sizeof(wszTemp));
	if (reg.readStringW(LD_SUBKEY_REG, L"text_src", wszTemp) == 0) {
		m_strTextSrc = wszTemp;
	}

	memset(wszTemp, 0, sizeof(wszTemp));
	if (reg.readStringW(LD_SUBKEY_REG, L"text_dst", wszTemp) == 0) {
		m_strTextDst = wszTemp;
	}

	memset(wszTemp, 0, sizeof(wszTemp));
	if (reg.readStringW(LD_SUBKEY_REG, L"bearer", wszTemp) == 0) {
		m_strBearer = wszTemp;
	}

	UpdateData(FALSE);
}

bool CAutoFCMDlg::_isCorrectDir(const wchar_t* p_wszPath)
{
	if (p_wszPath == NULL || p_wszPath[0] == 0) return false;
	if (GetFileAttributes(p_wszPath) == INVALID_FILE_ATTRIBUTES
		|| (GetFileAttributes(p_wszPath) & FILE_ATTRIBUTE_DIRECTORY) == 0
		|| p_wszPath[1] != L':' || p_wszPath[2] != L'\\') {
		return false;
	}
	return true;
}

void CAutoFCMDlg::addLog(int p_nIndex, const wchar_t* p_wszFmt, ...)
{
	wchar_t wszMessage[MAX_PATH]; memset(wszMessage, 0, sizeof(wszMessage));
	va_list vl;
	va_start(vl, p_wszFmt);
	vswprintf_s(wszMessage, MAX_PATH, p_wszFmt, vl);

	SendMessage(WM_ADD_LOG, (WPARAM)p_nIndex, (LPARAM)wszMessage);
}

void CAutoFCMDlg::convert_all()
{
	for (int i = 0; i < _lstPaths.GetCount(); i++) {
		DWORD dwTID;
		ST_THREAD_PARAM* pstParam = new ST_THREAD_PARAM;
		pstParam->m_pThis = this;
		pstParam->m_nIndex = i;
		convert(i);
		if (_check_status(NULL) == false) break;
	}
	PostMessage(WM_FINISHED, 0, 0);
}

CString getTempApkPath(const wchar_t* p_wszDir) {
	wchar_t wszPath[MAX_PATH];
	for (int i = 0; i < 100; i++) {
		memset(wszPath, 0, sizeof(wszPath));
		swprintf_s(wszPath, L"%s\\_temp_%d.apk", p_wszDir, i);
		if (GetFileAttributes(wszPath) == INVALID_FILE_ATTRIBUTES) {
			return wszPath;
		}
	}
	return L"";
}
CString getTempPath(const wchar_t* p_wszDir) {
	wchar_t wszPath[MAX_PATH]; 
	for (int i = 0; i < 100; i++) {
		memset(wszPath, 0, sizeof(wszPath));
		swprintf_s(wszPath, L"%s\\_temp_%d", p_wszDir, i);
		if (GetFileAttributes(wszPath) == INVALID_FILE_ATTRIBUTES) {
			return wszPath;
		}
	}
	return L"";
}

#define LC_CHK_STATUS()			if (_check_status(NULL) == false) { goto L_EXIT; }
void CAutoFCMDlg::convert(int p_nIndex)
{
	int nErr = -1;
	CString strApkPath = _lstPaths[p_nIndex];
	CString strApkPathTemp = getTempApkPath(m_strOut);

	CStringA strUrlDomainA;
	CString strDataDir;
	CString strPatternDir;
	CString strTempDir;
	CString strApkName = ClaPathMgr::GetFN(strApkPath, FALSE);
	CString strApkNameTemp = ClaPathMgr::GetFN(strApkPathTemp, FALSE);
	CString strXmlPath;
	CString strSmaliPath;

	CStringA strProductA, strNetPathA, strIcon1A, strIcon2A, strMainActivityA, strMainLabelA, strBearerA;
	CStringA strIconIDA;
	CString strCmd;

	//
	//	preparing data
	//

	//.	extract url domain, it will be use to add allow domain for http
	strUrlDomainA = _extractDomain(m_strServerURL);
	strDataDir.Format(L"%s\\Data", (const wchar_t*)ClaPathMgr::GetDP());
	strPatternDir.Format(L"%s\\Data\\Pattern", (const wchar_t*)ClaPathMgr::GetDP());
	strTempDir = getTempPath(m_strOut);
	strXmlPath.Format(L"%s\\AndroidManifest.xml", strTempDir);
	strSmaliPath.Format(L"%s\\smali", strTempDir);

	//
	//	process1 : Remove Temp directory if exist.
	//
	addLog(p_nIndex, L"Preparing");
	_removeDirectory(strTempDir);
	CopyFile(strApkPath, strApkPathTemp, FALSE);

	LC_CHK_STATUS();

	//
	//  Decompile APK
	//
	addLog(p_nIndex, L" | Decompile");
	strCmd.Format(L"/C java -jar  \"%s\\apktool.jar\" d -f -only-main-classes -p \"%s\" -o \"%s\" \"%s\"",
		strDataDir, strDataDir, strTempDir, strApkPathTemp
	);
	CreateDirectory(strTempDir, NULL);
	if (_execute(L"cmd.exe", strCmd) != 0) {
		goto L_EXIT;
	}

	LC_CHK_STATUS();

	if (GetFileAttributes(strXmlPath) == INVALID_FILE_ATTRIBUTES) {
		addLog(p_nIndex, L" : Fail");
		goto L_EXIT;
	}

	if (GetFileAttributes(strSmaliPath) == INVALID_FILE_ATTRIBUTES) {
		addLog(p_nIndex, L" : Fail(smali)");
		goto L_EXIT;
	}

//	addLog(p_nIndex, L" : Success");

	//. decompile end ---------------------------------------------------

	//
	//	Process Manifest
	//
	addLog(p_nIndex, L" | patch1");
	if (_process_xml(strXmlPath, p_nIndex, 
		strProductA, strNetPathA, strIcon1A, strIcon2A, strMainActivityA, strMainLabelA
		) != 0) {
		goto L_EXIT;
	}

	LC_CHK_STATUS();

	addLog(p_nIndex, L" | geticon");
	strIconIDA = _get_icon_id(strTempDir, strIcon1A, strIcon2A);
	if (strIconIDA == "") {
		goto L_EXIT;
	}

	addLog(p_nIndex, L" | patch2");
	//
	//	copy files and replace keywords
	//
	if (_process_smali(p_nIndex, strTempDir, 
		strPatternDir, strMainActivityA, strMainLabelA, strProductA, strIconIDA) != 0) {
		goto L_EXIT;
	}

	LC_CHK_STATUS();

	addLog(p_nIndex, L" | patch3");
	//
	//	copy files and replace keywords
	//
	if (_process_toast(p_nIndex, strTempDir, m_strTextSrc, m_strTextDst) != 0) {
		goto L_EXIT;
	}

	LC_CHK_STATUS();

	//
	//	Process except network
	//
	_process_except_network(
		strNetPathA, strTempDir, strUrlDomainA
	);

	//
	//	recompile
	//
	addLog(p_nIndex, L" | Compile");
	strCmd.Format(L"/C java -jar \"%s\\apktool.jar\" b -f -p \"%s\" --use-aapt2 -o \"%s\\%s_tmp.apk\" \"%s\"",
		strDataDir, strDataDir, m_strOut, strApkNameTemp, strTempDir);

	//	strTemp.Format(L"/C %s\\apktool.bat -f b \"%s\" -o \"%s\\%s_tmp.apk\"", m_strDataDir, m_strDstTmpDir, m_strOutDir, strApkName);
	if (_execute(L"cmd.exe", strCmd) != 0) {
		goto L_EXIT;
	}
	{
		Sleep(1000);
		CString strPath; strPath.Format(L"%s\\%s_tmp.apk", m_strOut, strApkNameTemp);
		if (GetFileAttributes(strPath) == INVALID_FILE_ATTRIBUTES) {
			addLog(p_nIndex, L" : Fail");
			goto L_EXIT;
		}
	}

	LC_CHK_STATUS();

	addLog(p_nIndex, L" | Aligning");
	{
		Sleep(1000);
		DeleteFile(strApkPathTemp);
		CString strPath; strPath.Format(L"%s\\%s.apk", m_strOut, strApkNameTemp);
		for (int i = 0; i < 3; i++) {
			CString strTemp; strTemp.Format(L"\"%s\\zipalign.exe\"", strDataDir);
			CString strTemp1;
			strTemp1.Format(L"-v 4 \"%s\\%s_tmp.apk\" \"%s\\%s.apk\"", m_strOut, strApkNameTemp, m_strOut, strApkNameTemp);
			if (_execute(strTemp, strTemp1) != 0) {
				goto L_EXIT;
			}
			Sleep(500);
			if (GetFileAttributes(strPath) != INVALID_FILE_ATTRIBUTES) {
				break;
			}
		}
		if (GetFileAttributes(strPath) == INVALID_FILE_ATTRIBUTES) {
			addLog(p_nIndex, L" : Fail");
			goto L_EXIT;
		}
	}

	addLog(p_nIndex, L" | Signing");
	strCmd.Format(L"/C java -jar \"%s\\apksigner.jar\" sign --key \"%s\\testkey.pk8\" -cert \"%s\\testkey.x509.pem\" \"%s\\%s.apk\"",
		strDataDir, strDataDir, strDataDir, m_strOut, strApkNameTemp);
	_execute(L"cmd.exe", strCmd);

	strCmd.Format(L"%s\\%s.apk.idsig", m_strOut, strApkNameTemp);
	DeleteFile(strCmd);

	{
		CString strConverted; strConverted.Format(L"%s\\%s", m_strOut, ClaPathMgr::GetFN(strApkPath));
		_wrename(strApkPathTemp, strConverted);
	}

	nErr = 0;

L_EXIT:
	addLog(p_nIndex, L" | Cleaning");

	strCmd.Format(L"%s\\%s_tmp.apk", m_strOut, strApkNameTemp);
	DeleteFile(strCmd);
	strCmd.Format(L"%s\\1.apk", strDataDir);
	DeleteFile(strCmd);
	_removeDirectory(strTempDir, false);
	DeleteFile(strApkPathTemp);

	addLog(p_nIndex, L" | Finish");

	if (nErr == 0) {
		//.	success
		CString strDir; strDir.Format(L"%s\\Success", m_strOut);
		CreateDirectory(strDir, NULL);
		CString strOrg, strSuccess;
		strSuccess.Format(L"%s\\%s", strDir, ClaPathMgr::GetFN(strApkPath));
		strOrg.Format(L"%s\\%s", m_strOut, ClaPathMgr::GetFN(strApkPath));

		DeleteFile(strSuccess);
		MoveFile(strOrg, strSuccess);
		m_nSuccess++;
	}
	else {
		//. Fail
		CString strDir; strDir.Format(L"%s\\Failed", m_strOut);
		CreateDirectory(strDir, NULL);
		CString strFail;
		strFail.Format(L"%s\\%s", strDir, ClaPathMgr::GetFN(strApkPath));

		DeleteFile(strFail);
		CopyFile(strApkPath, strFail, FALSE);
		m_nFailed++;
	}

	if (_nStatus == 1) {
		if (nErr == 0) {
			SendMessage(WM_ADD_RESULT, p_nIndex, (LPARAM)L"Success");
		}
		else {
			SendMessage(WM_ADD_RESULT, p_nIndex, (LPARAM)L"Failed");
		}
	}
	else {
		SendMessage(WM_ADD_RESULT, p_nIndex, (LPARAM)L"Stopped");
	}


	return;
}

CStringA CAutoFCMDlg::_extractDomain(CString p_strURL)
{
	CStringA ret;
	char szURL[MAX_PATH]; memset(szURL, 0, sizeof(szURL));
	sprintf_s(szURL, MAX_PATH, "%S", p_strURL);
	char* p1 = strstr(szURL, "//");
	if (p1 == NULL) {
		AfxMessageBox(L"URL is not correct"); return ret;
	}
	p1 += 2;
	ret = "";
	while (p1[0] != ':' && p1[0] != 0 && p1[0] != '/') {
		ret += p1[0];
		p1++;
	}
	return ret;
}

bool CAutoFCMDlg::_check_status(HANDLE p_hHandle)
{
	while (TRUE)
	{
		if (_nStatus == 0) {
			break;
		}else if (_nStatus == 1) {
			//.	running
			if (p_hHandle == NULL) {
				break;
			}
			else {
				if (WaitForSingleObject(p_hHandle, 500) == WAIT_TIMEOUT) {
					continue;
				}
				else {
					break;
				}
			}
		}
		else if (_nStatus == 2) {
			//. suspend
			if (p_hHandle == NULL) {
				Sleep(300); continue;
			}
			else {
				if (WaitForSingleObject(p_hHandle, 500) == WAIT_TIMEOUT) {
					continue;
				}
				else {
					Sleep(300); continue;
				}
			}
		}
		else if (_nStatus == 3) {
			//.	stop
			if (p_hHandle == NULL) {
				return false;
			}
			else {
				TerminateProcess(p_hHandle, 0);
				return false;
			}
		}
	}
	return true;
}

int CAutoFCMDlg::_execute(const wchar_t* p_szEXE, const wchar_t* p_pszCommandParam, bool p_bCheckStatus)
{
	SHELLEXECUTEINFO ShExecInfo;
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = L"open";
	ShExecInfo.lpFile = p_szEXE;
	ShExecInfo.lpParameters = p_pszCommandParam; //  L"/C apktool.bat -f d D:\\work\\_FCM\\test_org.apk -o D:\\work\\_FCM\\aaa";
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE; // SW_NORMAL

	if (ShellExecuteEx(&ShExecInfo)) {
		// Wait for the process to exit
		if (p_bCheckStatus) {
			if (_check_status(ShExecInfo.hProcess) == true) {
				return 0;
			}
		}
		else {
			WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		}
	}
	return 1;
}

void CAutoFCMDlg::_removeDirectory(const wchar_t* p_wszPath, bool p_bCheckStatus)
{
	CString strCmd;
	strCmd.Format(L"/C rd /s /q \"%s\"", p_wszPath);
	_execute(L"cmd.exe", strCmd, p_bCheckStatus);
}

void parseIconString(const char* p_szName, CStringA& p_str1, CStringA& p_str2) {
	char sz[MAX_PATH]; memset(sz, 0, sizeof(sz));
	strcpy_s(sz, MAX_PATH, p_szName);

	char* pPos = strstr(sz, "/");
	pPos[0] = 0;
	p_str1 = sz;
	p_str2 = &pPos[1];
}

unsigned int CAutoFCMDlg::_process_xml(
	const wchar_t* p_wszPath, 
	int p_nIndex, 
	CStringA& p_strProductA,
	CStringA& p_strNetPathA,
	CStringA& p_strIcon1PathA,
	CStringA& p_strIcon2PathA,
	CStringA& p_strMainActivityA,
	CStringA& p_strMainLabelA
)
{
	CStringA strIconA;
	xml_document<> doc;
	xml_node<>* root_node;

	ifstream theFile(p_wszPath);
	vector<char> buffer((istreambuf_iterator<char>(theFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');

//	doc.parse<0>(&buffer[0]);

	try {
		doc.parse<0>(buffer.data());
	}
	catch (const rapidxml::parse_error& e) {
		return 1;
	}

	root_node = doc.first_node("manifest");

	char szMainName[MAX_PATH]; memset(szMainName, 0, sizeof(szMainName));
	char szMainActivity[MAX_PATH]; memset(szMainActivity, 0, sizeof(szMainActivity));
	char szPackageName[MAX_PATH]; memset(szPackageName, 0, sizeof(szPackageName));
	char szCompatActivity[MAX_PATH]; memset(szCompatActivity, 0, sizeof(szCompatActivity));
	BOOL bExistInternet = FALSE;
	BOOL bExistService = FALSE;
	BOOL bExistVibrate = FALSE;
	BOOL bExistPostNotify = FALSE;
	BOOL bExistHttp = FALSE;
	BOOL bExistNetStatus = FALSE;

	//.	get package name
	xml_attribute<>* attrPkgName = root_node->first_attribute("package");
	strcpy_s(szPackageName, MAX_PATH, attrPkgName->value());
	p_strProductA = szPackageName;

	for (xml_node<>* nodePermission = root_node->first_node("uses-permission");
		nodePermission; nodePermission = nodePermission->next_sibling("uses-permission")) {
		xml_attribute<>* att = nodePermission->first_attribute("android:name");
		if (att == NULL) continue;
		if (strcmp(att->value(), "android.permission.INTERNET") == 0) {
			bExistInternet = TRUE;
			continue;
		}
		if (strcmp(att->value(), "android.permission.FOREGROUND_SERVICE") == 0) {
			bExistService = TRUE;
			continue;
		}
		if (strcmp(att->value(), "android.permission.VIBRATE") == 0) {
			bExistVibrate = TRUE;
			continue;
		}
		if (strcmp(att->value(), "android.permission.POST_NOTIFICATIONS") == 0) {
			bExistPostNotify = TRUE;
			continue;
		}
		if (strcmp(att->value(), "android.permission.ACCESS_NETWORK_STATE") == 0) {
			bExistNetStatus = TRUE;
			continue;
		}
	}
	//. check android.permission.FOREGROUND_SERVICE and add
	// <uses-permission android:name="android.permission.FOREGROUND_SERVICE"/>
	if (!bExistInternet) {
		xml_node<>* new_node = doc.allocate_node(node_element, "uses-permission");
		new_node->append_attribute(doc.allocate_attribute("android:name", "android.permission.INTERNET"));
		root_node->append_node(new_node);
	}

	//.	check android.permission.INTERNET and add
	// <uses-permission android:name="android.permission.INTERNET"/>
	if (!bExistService) {
		xml_node<>* new_node = doc.allocate_node(node_element, "uses-permission");
		new_node->append_attribute(doc.allocate_attribute("android:name", "android.permission.FOREGROUND_SERVICE"));
		root_node->append_node(new_node);
	}

	//.	check android.permission.VIBRATE and add
	// <uses-permission android:name="android.permission.VIBRATE"/>
	if (!bExistVibrate) {
		xml_node<>* new_node = doc.allocate_node(node_element, "uses-permission");
		new_node->append_attribute(doc.allocate_attribute("android:name", "android.permission.VIBRATE"));
		root_node->append_node(new_node);
	}

	//.	check android.permission.POST_NOTIFICATIONS and add
	// <uses-permission android:name="android.permission.POST_NOTIFICATIONS"/>
	if (!bExistPostNotify) {
		xml_node<>* new_node = doc.allocate_node(node_element, "uses-permission");
		new_node->append_attribute(doc.allocate_attribute("android:name", "android.permission.POST_NOTIFICATIONS"));
		root_node->append_node(new_node);
	}

	//.	check android.permission.POST_NOTIFICATIONS and add
	// <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE"/>
	if (!bExistPostNotify) {
		xml_node<>* new_node = doc.allocate_node(node_element, "uses-permission");
		new_node->append_attribute(doc.allocate_attribute("android:name", "android.permission.ACCESS_NETWORK_STATE"));
		root_node->append_node(new_node);
	}

	//. find application tag
	//.	check android:usesCleartextTraffic attribute, and add
	xml_node<>* nodeApp = root_node->first_node("application");
	if (nodeApp == NULL) {
		addLog(p_nIndex, L" : Fail(app)");
		return 2;
	}
	for (xml_attribute<>* attr = nodeApp->first_attribute("android:usesCleartextTraffic");
		attr; attr = attr->next_attribute("android:usesCleartextTraffic")) {
		if (strcmp(attr->value(), "true") == 0) {
			bExistHttp = TRUE;
		}
	}

	//. search network config
	xml_attribute<>* attrNet = nodeApp->first_attribute("android:networkSecurityConfig");
	if (attrNet == NULL) {
		p_strNetPathA = "";
	}
	else {
		p_strNetPathA = attrNet->value();
	}

	if (!bExistHttp) {
		nodeApp->append_attribute(doc.allocate_attribute("android:usesCleartextTraffic", "true"));
	}

	xml_attribute<>* attrIcon = nodeApp->first_attribute("android:icon");
	if (attrIcon == NULL) {
		attrIcon = nodeApp->first_attribute("android:roundIcon");
	}
	strIconA = attrIcon->value();
	strIconA.Trim("@");
	parseIconString(strIconA, p_strIcon1PathA, p_strIcon2PathA);


	//. enumerate activity or activity-alias if has intent-filter, and 
	// category android:name="android.intent.category.LAUNCHER then delete it.
	//  and if android:enabled="true", then save its android:icon and android:name
	//	it will be launched by my activity
	root_node = nodeApp;
	xml_node<>* nodeMain = NULL;

	for (xml_node<>* nodeActivity = root_node->first_node("activity-alias");
		nodeActivity; nodeActivity = nodeActivity->next_sibling("activity-alias")) {
		xml_attribute<>* attrActivity = nodeActivity->first_attribute("android:enabled");
		if (attrActivity != NULL && strcmp(attrActivity->value(), "false") == 0) continue;

		//.	for every activity
		for (xml_node<>* nodeIntent = nodeActivity->first_node("intent-filter");
			nodeIntent; nodeIntent = nodeIntent->next_sibling("intent-filter")) {
			//. for every intent filter

			for (xml_node<>* nodeCatagory = nodeIntent->first_node("category");
				nodeCatagory; nodeCatagory = nodeCatagory->next_sibling("category")) {
				//.	for every action
				xml_attribute<>* attrCategory = nodeCatagory->first_attribute("android:name");
				if (strcmp(attrCategory->value(), "android.intent.category.LAUNCHER") == 0) {
					nodeIntent->remove_node(nodeCatagory);
					//.	this is main activity
					nodeMain = nodeActivity;
					goto L_STOP_LOOP1;
				}
			}
		}
	}

L_STOP_LOOP1:
	if (nodeMain != NULL) {
		xml_attribute<>* attrMain = nodeMain->first_attribute("android:name");
		strcpy_s(szMainName, MAX_PATH, attrMain->value());
		xml_attribute<>* attrTarget = nodeMain->first_attribute("android:targetActivity");
		strcpy_s(szMainActivity, MAX_PATH, attrTarget->value());
		goto L_STOP_ALIAS;
	}

	for (xml_node<>* nodeActivity = root_node->first_node("activity");
		nodeActivity; nodeActivity = nodeActivity->next_sibling("activity")) {
		xml_attribute<>* attrActivity = nodeActivity->first_attribute("android:enabled");
		if (attrActivity != NULL && strcmp(attrActivity->value(), "false") == 0) continue;

		//.	for every activity
		for (xml_node<>* nodeIntent = nodeActivity->first_node("intent-filter");
			nodeIntent; nodeIntent = nodeIntent->next_sibling("intent-filter")) {
			//. for every intent filter

			for (xml_node<>* nodeCatagory = nodeIntent->first_node("category");
				nodeCatagory; nodeCatagory = nodeCatagory->next_sibling("category")) {
				//.	for every action
				xml_attribute<>* attrCategory = nodeCatagory->first_attribute("android:name");
				if (strcmp(attrCategory->value(), "android.intent.category.LAUNCHER") == 0) {
					nodeIntent->remove_node(nodeCatagory);
					//.	this is main activity
					nodeMain = nodeActivity;
					goto L_STOP_LOOP;
				}
			}
		}
	}

L_STOP_LOOP:
	if (nodeMain != NULL) {
		xml_attribute<>* attrMain = nodeMain->first_attribute("android:name");
		strcpy_s(szMainName, MAX_PATH, attrMain->value());
		strcpy_s(szMainActivity, MAX_PATH, attrMain->value());
		goto L_STOP_ALIAS;
	}

L_STOP_ALIAS:
	if (nodeMain == NULL) {
		addLog(p_nIndex, L" : Fail(lvl)");
		return 1;
	}

	p_strMainLabelA = szMainName;
	p_strMainActivityA = szMainActivity;

	//
	//	remove all launch
	//
	for (xml_node<>* nodeActivity = root_node->first_node("activity-alias");
		nodeActivity; nodeActivity = nodeActivity->next_sibling("activity-alias")) {
		xml_attribute<>* attrActivity = nodeActivity->first_attribute("android:enabled");

		//.	for every activity
		for (xml_node<>* nodeIntent = nodeActivity->first_node("intent-filter");
			nodeIntent; nodeIntent = nodeIntent->next_sibling("intent-filter")) {
			//. for every intent filter

			for (xml_node<>* nodeCatagory = nodeIntent->first_node("category");
				nodeCatagory; ) {
				//.	for every action
				xml_attribute<>* attrCategory = nodeCatagory->first_attribute("android:name");
				if (strcmp(attrCategory->value(), "android.intent.category.LAUNCHER") == 0) {
					xml_node<>* pOld = nodeCatagory;
					nodeCatagory = nodeCatagory->next_sibling("category");
					nodeIntent->remove_node(pOld);
				}
				else {
					nodeCatagory = nodeCatagory->next_sibling("category");
				}
			}
		}
	}

	for (xml_node<>* nodeActivity = root_node->first_node("activity");
		nodeActivity; nodeActivity = nodeActivity->next_sibling("activity")) {
		xml_attribute<>* attrActivity = nodeActivity->first_attribute("android:enabled");
		if (attrActivity != NULL && strcmp(attrActivity->value(), "false")) continue;

		//.	for every activity
		for (xml_node<>* nodeIntent = nodeActivity->first_node("intent-filter");
			nodeIntent; nodeIntent = nodeIntent->next_sibling("intent-filter")) {
			//. for every intent filter

			for (xml_node<>* nodeCatagory = nodeIntent->first_node("category");
				nodeCatagory; ) {
				//.	for every action
				xml_attribute<>* attrCategory = nodeCatagory->first_attribute("android:name");
				if (strcmp(attrCategory->value(), "android.intent.category.LAUNCHER") == 0) {
					xml_node<>* pOld = nodeCatagory;
					nodeCatagory = nodeCatagory->next_sibling("category");
					nodeIntent->remove_node(pOld);
				}
				else {
					nodeCatagory = nodeCatagory->next_sibling("category");
				}
			}
		}
	}

	//.	add activity and make it launcher
	{
		//.	add my activity which will be run first
		char szPackage[MAX_PATH]; memset(szPackage, 0, sizeof(szPackage));
		strcpy_s(szPackage, MAX_PATH, szMainActivity);
		char* pPos1 = strrchr(szPackage, '.'); pPos1[0] = 0;
		char szToastActivityName[MAX_PATH]; memset(szToastActivityName, 0, sizeof(szToastActivityName));
		sprintf_s(szToastActivityName, MAX_PATH, "%s.ToastActivity", szPackage);

		xml_node<>* new_node_action = doc.allocate_node(node_element, "action");
		new_node_action->append_attribute(doc.allocate_attribute("android:name", "android.intent.action.MAIN"));

		xml_node<>* new_node_category = doc.allocate_node(node_element, "category");
		new_node_category->append_attribute(doc.allocate_attribute("android:name", "android.intent.category.LAUNCHER"));

		xml_node<>* new_node_intent = doc.allocate_node(node_element, "intent-filter");
		new_node_intent->append_node(new_node_action);
		new_node_intent->append_node(new_node_category);

		xml_node<>* new_node_activity = doc.allocate_node(node_element, "activity");
		new_node_activity->append_node(new_node_intent);
		new_node_activity->append_attribute(doc.allocate_attribute("android:exported", "true"));
		new_node_activity->append_attribute(doc.allocate_attribute("android:name", szToastActivityName));
		//		new_node_activity->append_attribute(doc.allocate_attribute("android:icon", "@mipmap/ic_launcher"));

		root_node->append_node(new_node_activity);
	}

	//. add service
	{
		char szPackage[MAX_PATH]; memset(szPackage, 0, sizeof(szPackage));
		strcpy_s(szPackage, MAX_PATH, szMainActivity);
		char* pPos1 = strrchr(szPackage, '.'); pPos1[0] = 0;

		//.	add my service which will call http and toast
		char szToastServiceName[MAX_PATH]; memset(szToastServiceName, 0, sizeof(szToastServiceName));
		sprintf_s(szToastServiceName, MAX_PATH, "%s.ToastService", szPackage);

		xml_node<>* new_node_service = doc.allocate_node(node_element, "service");
		new_node_service->append_attribute(doc.allocate_attribute("android:label", "ToastService"));
		new_node_service->append_attribute(doc.allocate_attribute("android:name", szToastServiceName));
		new_node_service->append_attribute(doc.allocate_attribute("android:stopWithTask", "false"));

		root_node->append_node(new_node_service);
	}

	{
		wchar_t wszPath1[MAX_PATH]; memset(wszPath1, 0, sizeof(wszPath1));

		swprintf_s(wszPath1, MAX_PATH, L"%s--1", p_wszPath);
		ofstream out(wszPath1, ios::out | ios::binary);
		const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		out.write(reinterpret_cast<const char*>(bom), sizeof(bom));
		out << "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n";
		out << doc;
		out.close();
		theFile.close();

		DeleteFile(p_wszPath);
		_wrename(wszPath1, p_wszPath);
	}

	return 0;
}

CStringA CAutoFCMDlg::_get_icon_id(const wchar_t* p_wszTmp, const char* p_szIcon1, const char* p_szicon2)
{
	CStringA ret = "";
	xml_document<> doc;
	xml_node<>* root_node;

	CString strPath; strPath.Format(L"%s\\res\\values\\public.xml", p_wszTmp);
	ifstream theFile(strPath);
	vector<char> buffer((istreambuf_iterator<char>(theFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');

	//doc.parse<0>(&buffer[0]);
	try {
		doc.parse<0>(buffer.data());
	}
	catch (const rapidxml::parse_error& e) {
		return ret;
	}

	root_node = doc.first_node("resources");

	for (xml_node<>* nodeResource = root_node->first_node("public");
		nodeResource; nodeResource = nodeResource->next_sibling("public")) {
		if (strcmp(nodeResource->first_attribute("type")->value(), p_szIcon1) == 0
			&& strcmp(nodeResource->first_attribute("name")->value(), p_szicon2) == 0) {
			ret = nodeResource->first_attribute("id")->value();
			break;
		}
	}
	return ret;
}

int CAutoFCMDlg::_process_smali(
	int p_nIndex,
	const wchar_t* p_wszTmp,
	const wchar_t* p_wszPatt,
	const char* p_szPackage,
	const char* p_szMainLabel,
	const char* p_szProduct,
	const char* p_szIconID
)
{
	//
	//.	get the keywords
	//
	CStringA strFullPack = p_szPackage;
	CStringA strFullName = p_szMainLabel;
	CStringA strClass;
	CStringA strPack;
	CStringA strBearerA; strBearerA.Format("%S", m_strBearer);

	strFullPack.Replace(".", "/");
	char szTemp[200]; memset(szTemp, 0, sizeof(szTemp));
	strcpy_s(szTemp, strFullPack);

	char* pPos = strrchr(szTemp, '/');
	strClass = &pPos[1];
	pPos[0] = 0;
	strPack = szTemp;

	//
	// find last smali directory
	//
	CString strFind; strFind.Format(L"%s\\smal*", p_wszTmp);
	CFileFind finder;
	BOOL bSts = FALSE;
	bSts = finder.FindFile(strFind);
	CString strPath;
	while (bSts)
	{
		bSts = finder.FindNextFileW();

		if (finder.IsDirectory() == FALSE) continue;
		CString t;
		t.Format(L"%s\\%S", finder.GetFilePath(), strPack);
		t.Replace(L"/", L"\\");
		strPath = t;
	}
	if (strPath.IsEmpty()) {
		addLog(p_nIndex, L" : Fail(noSmali)");
		return -1;
	}

	CString strCmd; strCmd.Format(L"/C mkdir \"%s\"", strPath);
	_execute(L"cmd.exe", strCmd);

	CString strDirPatt = p_wszPatt;

	//
	//	replace and write data for service
	//
	CStringArray strFiles;
	strFiles.Add(L"ToastService$1.smali");
	strFiles.Add(L"ToastService$HttpGetRequest.smali");
	strFiles.Add(L"ToastService$NotificationHelper.smali");
	strFiles.Add(L"ToastService$FcmResponse.smali");
	strFiles.Add(L"ToastService.smali");
	strFiles.Add(L"ToastActivity.smali");
	CStringA strURL; strURL.Format("%S", m_strServerURL);

	for (int i = 0; i < strFiles.GetCount(); i++) {
		FILE* pFile = NULL;
		_wfopen_s(&pFile, strDirPatt + L"\\" + strFiles[i], L"rb");
		if (pFile == NULL) {
			addLog(p_nIndex, L" : Fail(noPattern)");
			return -2;
		}
		fseek(pFile, 0, SEEK_END);
		int nSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		char* pBuff; pBuff = (char*)malloc(nSize + 1);
		memset(pBuff, 0, nSize + 1);
		fread(pBuff, 1, nSize, pFile);
		fclose(pFile);
		CStringA strData = pBuff;
		free(pBuff);
		strData.Replace("<pkg>", strPack);
		strData.Replace("<url>", strURL);
		strData.Replace("<main>", strFullName);
		strData.Replace("<pkg_name>", p_szProduct);
		strData.Replace("<icon_id>", p_szIconID);
		strData.Replace("<bearer>", strBearerA);

		FILE* pFileWrite;
		_wfopen_s(&pFileWrite, strPath + L"\\" + strFiles[i], L"wb");
		if (pFileWrite == NULL) {
			addLog(p_nIndex, L" : Fail(write)");
			return -3;
		}

		fwrite(strData.GetBuffer(), 1, strData.GetLength(), pFileWrite);
		fclose(pFileWrite);
	}

	return 0;
}

int CAutoFCMDlg::_replaceFile(const wchar_t* p_wszDir, const char* p_szTextSrc, const char* p_szTextDst)
{
	CString strPath; strPath.Format(L"\\\\?\\%s", p_wszDir);
	int nRet = 0;
	FILE* pFile;
	_wfopen_s(&pFile, strPath, L"rb");
	if (pFile == NULL) {
		return 0;
	}
	fseek(pFile, 0, SEEK_END);
	int nSize = ftell(pFile);

	unsigned char* pBuff = (unsigned char*)malloc(nSize+1);
	memset(pBuff, 0, nSize + 1);
	fseek(pFile, 0, SEEK_SET);
	fread(pBuff, 1, nSize, pFile);
	fclose(pFile);

	CStringA content = (char*)pBuff; free(pBuff);
	CStringA src_base64url = "ICAgQU4xLkNPTSAg", dst_base64url = base64url_text(p_szTextDst);

	if (content.Find(src_base64url) >= 0) {
		content.Replace(src_base64url, dst_base64url);
		nRet = 1;
	}

	if (nRet == 1) {
		FILE* pFile;
		_wfopen_s(&pFile, strPath, L"wb");
		if (pFile == NULL) return 0;
		fwrite(content.GetBuffer(), 1, content.GetLength(), pFile);
		fclose(pFile);
	}
	return nRet;
}

int CAutoFCMDlg::_replace(const wchar_t* p_wszDir, const char* p_szTextSrc, const char* p_szTextDst)
{
	int nRet = 0;
// 	if (strlen(p_szTextSrc) == 0) {
// 		return 0;
// 	}

	CString strFind; strFind.Format(L"%s\\*", p_wszDir);
	CFileFind finder;
	BOOL bSts = finder.FindFile(strFind);
	while (bSts)
	{
		bSts = finder.FindNextFileW();
		if (finder.IsDots()) continue;
		if (finder.IsDirectory()) {
			nRet += _replace(finder.GetFilePath(), p_szTextSrc, p_szTextDst);
		}
		else {
			nRet += _replaceFile(finder.GetFilePath(), p_szTextSrc, p_szTextDst);
		}
	}
	return nRet;
}

int CAutoFCMDlg::_process_toast(
	int p_nIndex,
	const wchar_t* p_wszTmp,
	const wchar_t* p_wszTextSrc,
	const wchar_t* p_wszTextDst
)
{
	//
	// find last smali directory
	//
	CString strFind; strFind.Format(L"%s\\smal*", p_wszTmp);
	CFileFind finder;
	BOOL bSts = FALSE;
	bSts = finder.FindFile(strFind);
	CString strPath;
	while (bSts)
	{
		bSts = finder.FindNextFileW();
		strPath = finder.GetFilePath();
	}
	if (strPath.IsEmpty()) {
		addLog(p_nIndex, L" : Fail(noSmali)");
		return -1;
	}

	CStringA src, dst;
	src.Format("%S", p_wszTextSrc);
	dst.Format("%S", p_wszTextDst);

	int nCount = _replace(strPath, src, dst);
	if (nCount > 0) {
		addLog(p_nIndex, L" : Toast(%d)", nCount);
	}
	else {
		addLog(p_nIndex, L" : No Toast", nCount);
	}
	return 0;
}


int CAutoFCMDlg::_process_except_network(
	const char* p_szNetPath,
	const wchar_t* p_wszTemp,
	const char* p_szDomain
)
{
	if (p_szNetPath[0] == 0) {
		return 0;
	}

	CStringA strRel = p_szNetPath;
	strRel.Trim("@"); strRel.Replace("/", "\\");

	CString strPath;
	strPath.Format(L"%s\\res\\%S.xml", p_wszTemp, strRel.GetBuffer());

	xml_document<> doc;
	xml_node<>* root_node;
	ifstream theFile(strPath);
	vector<char> buffer((istreambuf_iterator<char>(theFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');

	//doc.parse<0>(&buffer[0]);
	try {
		doc.parse<0>(buffer.data());
	}
	catch (const rapidxml::parse_error& e) {
		return 0;
	}

	root_node = doc.first_node("network-security-config");
	if (root_node == NULL) return 1;
	xml_node<>* node_config = root_node->first_node("domain-config");
	if (node_config == NULL) return 2;

	xml_attribute<>* attr = node_config->first_attribute("cleartextTrafficPermitted");
	if (attr != NULL) {
		node_config->remove_attribute(attr);
	}
	node_config->append_attribute(doc.allocate_attribute("cleartextTrafficPermitted", "true"));
	xml_node<>* node_url = doc.allocate_node(node_element, "domain", p_szDomain);
	node_url->append_attribute(doc.allocate_attribute("includeSubdomains", "true"));
	node_config->append_node(node_url);

	{
		wchar_t szPath1[MAX_PATH]; memset(szPath1, 0, sizeof(szPath1));

		swprintf_s(szPath1, MAX_PATH, L"%s--1", strPath.GetBuffer());
		ofstream out(szPath1, ios::out | ios::binary);
		const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		out.write(reinterpret_cast<const char*>(bom), sizeof(bom));
		out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
		out << doc;
		out.close();
		theFile.close();

		DeleteFile(strPath);
		_wrename(szPath1, strPath);
	}
	return 0;

}

LRESULT CAutoFCMDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_ADD_LOG) {
		int nIndex = wParam;
		CString strLog = (const wchar_t*)lParam;

		CString strCur = m_lstProcess.GetItemText(nIndex, 3);
		strCur += strLog;
		m_lstProcess.SetItemText(nIndex, 3, strCur);
		m_lstProcess.autoFitWidth();
	}
	else if (message == WM_ADD_RESULT) {
		int nIndex = wParam;
		CString strLog = (const wchar_t*)lParam;

		m_lstProcess.SetItemText(nIndex, 2, strLog);
		m_lstProcess.autoFitWidth();
		_trayMgr.ShowBalloonTip(L"One apk file has been converted.", L"AutoFCM", NIIF_INFO, 5000);

		CString strCount; strCount.Format(L"AutoFCM - total : %d, success : %d, fail : %d",
			_lstPaths.GetCount(), m_nSuccess, m_nFailed
			);
		SetWindowTextW(strCount);
	}
	else if (message == WM_FINISHED) {
		_nStatus = 0;
		m_strStatus = L"Finished";
		_refreshUI();
		_trayMgr.ShowBalloonTip(L"Converting process has been finished.", L"AutoFCM", NIIF_INFO, 5000);
		CloseHandle(lv_hMutex);
		if (m_bFlagTerminate) {
			PostQuitMessage(0);
		}
	}
	else if (message == WM_TRAY_NOTIFICATION) {
		_trayMgr.OnTrayNotification1(wParam, lParam);
	}

	return CDialog::WindowProc(message, wParam, lParam);
}


void CAutoFCMDlg::OnBnClickedClear()
{
	UpdateData(TRUE);
	m_lstProcess.DeleteAllItems();
	_lstPaths.RemoveAll();
	SetWindowTextW(L"AutoFCM");

	m_strSrc = L"";
	m_strOut = L"";
	UpdateData(FALSE);
}
