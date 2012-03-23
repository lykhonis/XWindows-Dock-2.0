#ifndef OLEDATASOURCEEX_H
#define OLEDATASOURCEEX_H

#include <afxwin.h>
#include <afxdisp.h>
#include <afxole.h>
#include <shlobj.h>

class COleDataSourceEx: public COleDataSource
{
public:
	DECLARE_INTERFACE_MAP()
	
	BEGIN_INTERFACE_PART(DataObj, IDataObject)
		INIT_INTERFACE_PART(COleDataSourceEx, DataObj)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
	END_INTERFACE_PART(DataObj)
};

#endif /* OLEDATASOURCEEX_H */