#include "pch.h"
#include "ClaRegMgr.h"

void ClaRegMgr::createKey(LPCWSTR p_wszSubKey)
{
	HKEY newKey;
	LONG result = RegCreateKeyEx(_hRootKey, p_wszSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &newKey, nullptr);
	if (result != ERROR_SUCCESS) {
		
	}

	return;
}

unsigned long ClaRegMgr::writeStringW(const wchar_t* p_wszSubKey, const wchar_t* p_wszValueName, const wchar_t* p_wszValue)
{
	HKEY openedKey;
	LONG result = RegOpenKeyEx(_hRootKey, p_wszSubKey, 0, KEY_SET_VALUE, &openedKey);
	if (result != ERROR_SUCCESS) {
		return 1;
	}

	result = RegSetValueEx(openedKey, p_wszValueName, 0, REG_SZ, (const BYTE*)p_wszValue, (wcslen(p_wszValue) + 1) * sizeof(wchar_t));
	RegCloseKey(openedKey); // Close the key handle
	if (result != ERROR_SUCCESS) {
		return 2;
	}
	return 0;
}

unsigned long ClaRegMgr::readStringW(const wchar_t* p_wszSubKey, const wchar_t* p_wszValueName, wchar_t* p_wszValue)
{
	DWORD dataSize = 0;
	LONG result = RegGetValue(_hRootKey, p_wszSubKey, p_wszValueName, REG_MULTI_SZ, nullptr, nullptr, &dataSize);
	if (result != ERROR_SUCCESS) {
		return 1;
	}

	wchar_t* pTemp = (wchar_t*)malloc(dataSize + 2);
	memset(pTemp, 0, dataSize + 2);
	result = RegGetValue(_hRootKey, p_wszSubKey, p_wszValueName, REG_MULTI_SZ, nullptr, pTemp, &dataSize);
	if (result != ERROR_SUCCESS) {
		free(pTemp);
		return 2;
	}

	wcscpy_s(p_wszValue, MAX_PATH, pTemp);
	free(pTemp);
	return 0;
}
