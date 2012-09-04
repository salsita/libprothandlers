#include "StdAfx.h"

#include "TemporaryProtocolResourceHandlerClassFactory.h"

/*****************************************************************************
 * class CTemporaryProtocolResourceHandlerClassFactory
 *****************************************************************************/

//---------------------------------------------------------------------------
// FinalConstruct
HRESULT CTemporaryProtocolResourceHandlerClassFactory::FinalConstruct()
{
  return S_OK;
}

//---------------------------------------------------------------------------
// FinalRelease
void CTemporaryProtocolResourceHandlerClassFactory::FinalRelease()
{
}

//-------------------------------------------------------------------------
// AddHost
HRESULT CTemporaryProtocolResourceHandlerClassFactory::AddHost(
  LPCWSTR lpszHost,
  LPCWSTR lpszFileName)
{
  // load resource file
  // note that the library stays loaded until the process terminates
  HINSTANCE hInstResources =
        ::LoadLibraryEx(lpszFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
  if (!hInstResources)
  {
    DWORD dw = GetLastError();
    return HRESULT_FROM_WIN32(dw);
  }

  HRESULT hr = AddHost(lpszHost, hInstResources);
  if (FAILED(hr))
  {
    ::FreeLibrary(hInstResources);
  }
  return hr;
}

//---------------------------------------------------------------------------
// AddHost
HRESULT CTemporaryProtocolResourceHandlerClassFactory::AddHost(
  LPCWSTR   lpszHost,
  HINSTANCE hInstResources)
{
  CritSectLock lock(m_CriticalSection);

  // check arguments
  if (!wcslen(lpszHost) || !hInstResources)
  {
    return E_INVALIDARG;
  }

  // lookup host if we have already
  ResourceHandlerHostInfo hostInfo;
  if (LookupHostInfo(lpszHost, hostInfo))
  {
    // found
    return S_FALSE;
  }

  hostInfo.hostName = lpszHost;
  hostInfo.hResourceInstance = hInstResources;

  // add to map
  SetHostInfo(lpszHost, hostInfo);

  return S_OK;
}

//---------------------------------------------------------------------------
// InitHandler
HRESULT CTemporaryProtocolResourceHandlerClassFactory::InitHandler(
  CTemporaryProtocolResourceHandler * pHandler)
{
  return pHandler->Init(this);
}
