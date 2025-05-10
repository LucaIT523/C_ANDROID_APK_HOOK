#pragma once


class ClaPathMgr
{
public:
	ClaPathMgr() {};
	~ClaPathMgr() {};

public:
	static CString GetFN(const wchar_t* p_wszPath = NULL, BOOL p_bWithExt = TRUE);
	static CString GetDP(const wchar_t* p_wszPath = NULL);
};

