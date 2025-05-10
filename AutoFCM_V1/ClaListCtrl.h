#pragma once

#include <afxcmn.h>

class ClaListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(ClaListCtrl)

public:
	ClaListCtrl();

	void setHead(const wchar_t* headers);
	void addRecord(const wchar_t* firstItem, ...);
	void autoFitWidth();

protected:
	int _nColCnt;

protected:
	DECLARE_MESSAGE_MAP()
};

