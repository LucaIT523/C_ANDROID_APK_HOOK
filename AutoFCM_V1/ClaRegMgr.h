#pragma once
class ClaRegMgr
{
public:
	ClaRegMgr(HKEY p_hRootKey = HKEY_LOCAL_MACHINE) { _hRootKey = p_hRootKey; };
	~ClaRegMgr() {};

	void createKey(LPCWSTR p_wszSubKey);
protected:
	HKEY _hRootKey;

public:
	unsigned long writeStringW(const wchar_t* p_wszKey, const wchar_t* p_wszKeyName, const wchar_t* p_wszValue);
	unsigned long readStringW(const wchar_t* p_wszKey, const wchar_t* p_wszKeyName, wchar_t* p_wszValue);
};

