#include "StdAfx.h"
#include "TemporaryProtocolResourceHandler.h"
#include "TemporaryProtocolResourceHandlerClassFactory.h"

// CTOR ResourceHandlerHostInfo
ResourceHandlerHostInfo::ResourceHandlerHostInfo() :
  hResourceInstance(NULL)
{
}

/*****************************************************************************
 * class CTemporaryProtocolResourceHandler
 *****************************************************************************/
//---------------------------------------------------------------------------
// the CLSID for this object
// {5902FEC0-9341-47cd-AAD3-A63FD45E01D4}
const GUID CTemporaryProtocolResourceHandler::CLSID =
  {0x5902fec0, 0x9341, 0x47cd, {0xaa, 0xd3, 0xa6, 0x3f, 0xd4, 0x5e, 0x1, 0xd4}};

//---------------------------------------------------------------------------
// FreeResources
void CTemporaryProtocolResourceHandler::FreeResources()
{
  m_SpecialURLResource.clear();
  if (m_hGlobalResource)
  {
    FreeResource(m_hGlobalResource);
  }
  m_hrc = NULL;
  m_hGlobalResource = NULL;
  mData = NULL;
  mLength = 0;
  mPos = 0;
}

//---------------------------------------------------------------------------
// FinalConstruct
HRESULT CTemporaryProtocolResourceHandler::FinalConstruct()
{
  m_hrc = NULL;
  m_hGlobalResource = NULL;
  mData = NULL;
  mPos = mLength = 0;
  return S_OK;
}

//---------------------------------------------------------------------------
// FinalRelease
void CTemporaryProtocolResourceHandler::FinalRelease()
{
  FreeResources();
}

//---------------------------------------------------------------------------
// InitializeRequest
HRESULT CTemporaryProtocolResourceHandler::InitializeRequest(
  LPCWSTR lpszPath, DWORD & dwSize)
{
  // strip leading '/'
  CStringW sPath(lpszPath+1);

  // And adjust to resource name scheme.
  // In a resource name no slashes or backslashes are allowed. That's why we
  // use '|' as a path separator.
  sPath.Replace(_T('/'), _T('|'));

  m_hrc = FindResource(m_HostInfo.hResourceInstance, sPath, RT_HTML);
  if (NULL == m_hrc)
  {
    return INET_E_OBJECT_NOT_FOUND;
  }

  m_hGlobalResource = LoadResource(m_HostInfo.hResourceInstance, m_hrc);
  if (!m_hGlobalResource)
  {
    return INET_E_OBJECT_NOT_FOUND;
  }

  mPos = 0;
  mLength = dwSize = SizeofResource(m_HostInfo.hResourceInstance, m_hrc);
  mData = (LPBYTE)LockResource(m_hGlobalResource);
  return S_OK;
}

//---------------------------------------------------------------------------
// Read
STDMETHODIMP CTemporaryProtocolResourceHandler::Read(
  void *pv, ULONG cb, ULONG *pcbRead)
{
  if (m_SpecialURLResource.mData) {
    // have a special URL
    return m_SpecialURLResource.read(pv, cb, pcbRead);
  }
  return read(pv, cb, pcbRead);
}

//---------------------------------------------------------------------------
// UnlockRequest
STDMETHODIMP CTemporaryProtocolResourceHandler::UnlockRequest()
{
  FreeResources();
  return S_OK;
}

//---------------------------------------------------------------------------
// Seek
STDMETHODIMP CTemporaryProtocolResourceHandler::Seek(
  LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
  if (m_SpecialURLResource.mData) {
    // have a special URL
    return m_SpecialURLResource.seek(dlibMove, dwOrigin, plibNewPosition);
  }
  return seek(dlibMove, dwOrigin, plibNewPosition);
}
