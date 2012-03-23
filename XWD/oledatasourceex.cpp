#include "OleDataSourceEx.h"

BEGIN_INTERFACE_MAP(COleDataSourceEx, COleDataSource)
	INTERFACE_PART(COleDataSourceEx, IID_IDataObject, DataObj)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) COleDataSourceEx::XDataObj::AddRef()
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleDataSourceEx::XDataObj::Release()
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->ExternalRelease();
}

STDMETHODIMP COleDataSourceEx::XDataObj::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleDataSourceEx::XDataObj::GetData(LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->m_xDataObject.GetData(pFormatetc, pmedium);
}

STDMETHODIMP COleDataSourceEx::XDataObj::GetDataHere(LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->m_xDataObject.GetDataHere(pFormatetc, pmedium);
}

STDMETHODIMP COleDataSourceEx::XDataObj::QueryGetData(LPFORMATETC pFormatetc)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->m_xDataObject.QueryGetData(pFormatetc);
}

STDMETHODIMP COleDataSourceEx::XDataObj::GetCanonicalFormatEtc(LPFORMATETC pFormatetcIn, LPFORMATETC pFormatetcOut)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->m_xDataObject.GetCanonicalFormatEtc(pFormatetcIn, pFormatetcOut);
}

STDMETHODIMP COleDataSourceEx::XDataObj::SetData(LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium, BOOL fRelease)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)

	HRESULT hRes = pThis->m_xDataObject.SetData(pFormatetc, pmedium, fRelease);
	if(hRes == DATA_E_FORMATETC) 
	{
		pThis->CacheData(pFormatetc->cfFormat, pmedium, pFormatetc);
		return S_OK;
	}

	return hRes;
}

STDMETHODIMP COleDataSourceEx::XDataObj::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC* ppenumFormatetc)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->m_xDataObject.EnumFormatEtc(dwDirection, ppenumFormatetc);
}

STDMETHODIMP COleDataSourceEx::XDataObj::DAdvise(LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, LPDWORD pdwConnection)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->m_xDataObject.DAdvise(pFormatetc, advf, pAdvSink ,pdwConnection);
}

STDMETHODIMP COleDataSourceEx::XDataObj::DUnadvise(DWORD dwConnection)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->m_xDataObject.DUnadvise(dwConnection);
}

STDMETHODIMP COleDataSourceEx::XDataObj::EnumDAdvise(LPENUMSTATDATA* ppenumAdvise)
{
	METHOD_PROLOGUE(COleDataSourceEx, DataObj)
	return pThis->m_xDataObject.EnumDAdvise(ppenumAdvise);
}
