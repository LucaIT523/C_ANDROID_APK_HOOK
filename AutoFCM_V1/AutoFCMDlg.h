
// AutoFCMDlg.h : header file
//

#pragma once

#include "ClaListCtrl.h"
#include "ClaTrayIconMgr.h"

class CAutoFCMDlg;

typedef struct tagThreadParam
{
	CAutoFCMDlg* m_pThis;
	int m_nIndex;
}ST_THREAD_PARAM;

// CAutoFCMDlg dialog
class CAutoFCMDlg : public CDialog
{
// Construction
public:
	CAutoFCMDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTOFCM_V1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_strSrc;
	CString m_strOut;
	CString m_strServerURL;
	CString m_strLaunchButton;
	CString m_strLaunchURL;
	ClaListCtrl m_lstProcess;
	afx_msg void OnBnClickedBtnSrc();
	afx_msg void OnBnClickedBtnOut();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedCancel();

	afx_msg void OnTrayOpen();
	afx_msg void OnTrayExit();


protected:
	int _nStatus;

private:
	void _initUI();
	void _refreshUI();
	CStringArray _lstPaths;
	void _enumApkFiles(const wchar_t* p_wszPath);

private:
	void _saveSetting();
	void _loadSetting();
	bool _isCorrectDir(const wchar_t* p_wszPath);

public:
	void addLog(int p_nIndex, const wchar_t* p_wszFmt, ...);
	void convert_all();
	void convert(int p_nIndex);
	CStringA _extractDomain(CString p_strURL);
	bool _check_status(HANDLE p_hHandle);
	int _execute(const wchar_t* p_szEXE, const wchar_t* p_pszCommandParam, bool p_bCheckStatus = true);
	void _removeDirectory(const wchar_t* p_wszPath, bool p_bCheckStatus = true);
	unsigned int _process_xml(
		const wchar_t* p_wszPath,
		int p_nIndex,
		CStringA& p_strPackageA,
		CStringA& p_strNetPathA,
		CStringA& p_strIcon1PathA,
		CStringA& p_strIcon2PathA,
		CStringA& p_strMainActivityA,
		CStringA& p_strMainLabelA
		);
	CStringA _get_icon_id(const wchar_t* p_wszTmp, const char* p_szIcon1, const char* p_szicon2);
	int _process_smali(
		int p_nIndex,
		const wchar_t* p_wszTmp,
		const wchar_t* p_wszPatt,
		const char* p_szPackage,
		const char* p_szMainLabel,
		const char* p_szProduct,
		const char* p_szIconID
	);
	int _replaceFile(const wchar_t* p_wszDir, const char* p_szTextSrc, const char* p_szTextDst);
	int _replace(const wchar_t* p_wszDir, const char* p_szTextSrc, const char* p_szTextDst);
	int _process_toast(
		int p_nIndex,
		const wchar_t* p_wszTmp,
		const wchar_t* p_wszTextSrc,
		const wchar_t* p_wszTextDst
	);
	int _process_except_network(
		const char* p_szNetPath,
		const wchar_t* p_wszTemp,
		const char* p_szDomain
	);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	ClaTrayIconMgr _trayMgr;
	CString m_strStatus;

	int m_nSuccess;
	int m_nFailed;
	bool m_bFlagTerminate;
	afx_msg void OnBnClickedClear();
	CString m_strTextSrc;
	CString m_strTextDst;
	CString m_strBearer;
};
