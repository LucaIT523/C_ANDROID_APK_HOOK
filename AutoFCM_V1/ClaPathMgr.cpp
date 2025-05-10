#include "pch.h"
#include "ClaPathMgr.h"

CString ClaPathMgr::GetFN(const wchar_t* p_wszPath /*= NULL*/, BOOL p_bWithExt /*= TRUE*/)
{
	wchar_t wszPath[MAX_PATH]; memset(wszPath, 0, sizeof(wszPath));
	if (p_wszPath == NULL) {
		GetModuleFileName(NULL, wszPath, MAX_PATH);
	}
	else {
		wcscpy_s(wszPath, MAX_PATH, p_wszPath);
	}
	wchar_t* pPos = wcsrchr(wszPath, L'\\');

	if (pPos == NULL) return L"";

	if (!p_bWithExt) {
		wchar_t* pExt = wcsrchr(wszPath, L'.');
		if (pExt != NULL) pExt[0] = 0x0;
	}

	return &pPos[1];
}

CString ClaPathMgr::GetDP(const wchar_t* p_wszPath /*= NULL*/)
{
	wchar_t wszPath[MAX_PATH]; memset(wszPath, 0, sizeof(wszPath));
	if (p_wszPath == NULL) {
		GetModuleFileName(NULL, wszPath, MAX_PATH);
	}
	else {
		wcscpy_s(wszPath, MAX_PATH, p_wszPath);
	}
	wchar_t* pPos = wcsrchr(wszPath, L'\\');
	if (pPos != NULL) pPos[0] = 0;
	return wszPath;
}
