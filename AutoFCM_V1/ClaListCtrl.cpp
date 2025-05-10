#include "pch.h"
#include "ClaListCtrl.h"

#include <sstream>

IMPLEMENT_DYNAMIC(ClaListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(ClaListCtrl, CListCtrl)
END_MESSAGE_MAP()

ClaListCtrl::ClaListCtrl()
 : CListCtrl()
{

}

void ClaListCtrl::setHead(const wchar_t* headers)
{
	std::wstring headerString(headers);
	std::wistringstream iss(headerString);
	std::wstring header;
	int nCol = 0;
	while (std::getline(iss, header, L';'))
	{
		InsertColumn(nCol++, header.c_str(), LVCFMT_LEFT, 100); // Assuming a default width of 100
	}
	_nColCnt = nCol;
}

void ClaListCtrl::addRecord(const wchar_t* firstItem, ...)
{
	va_list args;
	va_start(args, firstItem);

	int nItem = GetItemCount();
	InsertItem(nItem, CString(firstItem));

	int nCol = 1;
	const wchar_t* nextItem = nullptr;
	while (nCol < _nColCnt)
	{
		nextItem = va_arg(args, const wchar_t*);
		SetItemText(nItem, nCol++, CString(nextItem));
	}

	va_end(args);
}

void ClaListCtrl::autoFitWidth()
{
    CHeaderCtrl* pHeader = GetHeaderCtrl();
    int nColumnCount = pHeader->GetItemCount();

    // Temporary variable to store the width needed for the header
    int nHeaderWidth = 0;
    HDITEM hdi = { 0 };
    hdi.mask = HDI_TEXT;
    TCHAR szText[256]; // Buffer to store the header text
    hdi.pszText = szText;
    hdi.cchTextMax = 255;

    // Go through each column
    for (int i = 0; i < nColumnCount; i++)
    {
        // Get the header item
        pHeader->GetItem(i, &hdi);

        // Set the width of the column to the width of the header text
        SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
        nHeaderWidth = GetColumnWidth(i);

        // Autosize the column width to the longest item's text width
        SetColumnWidth(i, LVSCW_AUTOSIZE);
        int nColumnWidth = GetColumnWidth(i);

        // Choose the maximum width between the header and the column content
        SetColumnWidth(i, max(nHeaderWidth, nColumnWidth));
    }
}

