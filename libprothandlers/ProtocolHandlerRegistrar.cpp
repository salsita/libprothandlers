#include "StdAfx.h"
#include "TemporaryProtocolFolderHandler.h"
#include "TemporaryProtocolFolderHandlerClassFactory.h"
#include "TemporaryProtocolResourceHandler.h"
#include "TemporaryProtocolResourceHandlerClassFactory.h"
#include "ProtocolHandlerRegistrar.h"

/*****************************************************************************
 * class CProtocolHandlerRegistrar
 *****************************************************************************/
CProtocolHandlerRegistrar & CProtocolHandlerRegistrar::GetInstance()
{
  static CProtocolHandlerRegistrar instance;
  return instance;
}

//---------------------------------------------------------------------------
// ctor
CProtocolHandlerRegistrar::CProtocolHandlerRegistrar(void)
{
  InitializeCriticalSection(&m_CriticalSection);
}

//---------------------------------------------------------------------------
// dtor
CProtocolHandlerRegistrar::~CProtocolHandlerRegistrar(void)
{
  DeleteCriticalSection(&m_CriticalSection);
}

//---------------------------------------------------------------------------
// RegisterTemporaryFolderHandler
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryFolderHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost,
  LPCWSTR lpszFolder)
{
  return GetInstance().
    RegisterTemporaryHandler<CTemporaryProtocolFolderHandlerClassFactory, LPCWSTR>
    (lpszScheme, lpszHost, lpszFolder);
}

//---------------------------------------------------------------------------
// RegisterTemporaryResourceHandler
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryResourceHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost,
  LPCWSTR lpszResourceFileName)
{
  return GetInstance().
    RegisterTemporaryHandler<CTemporaryProtocolResourceHandlerClassFactory, LPCWSTR>
    (lpszScheme, lpszHost, lpszResourceFileName);
}

//---------------------------------------------------------------------------
// RegisterTemporaryResourceHandler
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryResourceHandler(
  LPCWSTR   lpszScheme,
  LPCWSTR   lpszHost,
  HINSTANCE hInstResources)
{
  return GetInstance().
    RegisterTemporaryHandler<CTemporaryProtocolResourceHandlerClassFactory, HINSTANCE>
    (lpszScheme, lpszHost, hInstResources);
}

//---------------------------------------------------------------------------
// UnregisterTemporaryFolderHandler
HRESULT CProtocolHandlerRegistrar::UnregisterTemporaryFolderHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost)
{
  return GetInstance().
    UnregisterTemporaryHandler<CTemporaryProtocolFolderHandlerClassFactory>(lpszScheme, lpszHost);
}

//---------------------------------------------------------------------------
// UnregisterTemporaryResourceHandler
HRESULT CProtocolHandlerRegistrar::UnregisterTemporaryResourceHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost)
{
  return GetInstance().
    UnregisterTemporaryHandler<CTemporaryProtocolResourceHandlerClassFactory>(lpszScheme, lpszHost);
}

//---------------------------------------------------------------------------
// AddURL
HRESULT CProtocolHandlerRegistrar::AddURL(
  LPCWSTR lpszURL,
  LPCVOID lpData,
  DWORD dwLength,
  LPCWSTR lpszMimeType)
{
  return GetInstance().
    InternalAddURL(lpszURL, lpData, dwLength, lpszMimeType);
}

//---------------------------------------------------------------------------
// InternalAddURL
HRESULT CProtocolHandlerRegistrar::InternalAddURL(
  LPCWSTR lpszURL,
  LPCVOID lpData,
  DWORD dwLength,
  LPCWSTR lpszMimeType)
{
  // TODO: implement
return E_NOTIMPL;
  CComPtr<IUri> pURI;
  IF_FAILED_RET(::CreateUri(lpszURL, Uri_CREATE_CANONICALIZE, 0, &pURI));

  CritSectLock lock(m_CriticalSection);
  CComBSTR bsScheme;
  IF_FAILED_RET(pURI->GetSchemeName(&bsScheme));

  CComPtr<IClassFactory> pClassFactory;
  if (!m_ClassFactories.Lookup(bsScheme, pClassFactory))
  {
    return E_FAIL;
  }

  return S_OK;
}

//---------------------------------------------------------------------------
// RegisterTemporaryHandler
template<class CF, class RT>
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost,
  RT      tResourceID)
{
  CritSectLock lock(m_CriticalSection);
  BOOL registered = FALSE;

  CComObject<CF> *pHandlerFactory = NULL;
  CComPtr<IClassFactory> pClassFactory;

  // lookup class factory for lpszScheme
  if (!m_ClassFactories.Lookup(lpszScheme, pClassFactory))
  {
    // don't have yet, create class factory object
    IF_FAILED_RET(CComObject<CF>
      ::CreateInstance(&pHandlerFactory));

    pClassFactory = pHandlerFactory;

    // init classfactory object
    IF_FAILED_RET(pHandlerFactory->Init(
      lpszScheme));
  }
  else
  {
    // have class factory
    registered = TRUE;
    pHandlerFactory = (CComObject<CF> *)pClassFactory.p;
  }

  // add the host
  IF_FAILED_RET(pHandlerFactory->AddHost(
    lpszHost, tResourceID));

  // register protocol handler
  if (!registered)
  {
    // get IInternetSession
    CComPtr<IInternetSession> pInternetSession;
    IF_FAILED_RET(CoInternetGetSession(0, &pInternetSession, 0));

    IF_FAILED_RET(pInternetSession->RegisterNameSpace(
      pClassFactory,
      CTemporaryProtocolFolderHandler::CLSID,
      lpszScheme,
      0, NULL, 0));

    // store classfactory object
    m_ClassFactories[lpszScheme] = pClassFactory;
  }

  return S_OK;
}

//---------------------------------------------------------------------------
// UnregisterTemporaryHandler
template<class CF>
HRESULT CProtocolHandlerRegistrar::UnregisterTemporaryHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost)
{
  CritSectLock lock(m_CriticalSection);
  // lookup classfactory object for lpszHost
  CComPtr<IClassFactory> pClassFactory;
  if (!m_ClassFactories.Lookup(lpszScheme, pClassFactory))
  {
    // not found
    return S_FALSE;
  }

  // unregister host
  CComObject<CF> * pHandlerFactory = (CComObject<CF> *)pClassFactory.p;
  size_t registeredHosts = pHandlerFactory->RemoveHost(lpszHost);
  if (registeredHosts > 0)
  {
    // the handler has still registered hosts
    return S_OK;
  }

  // there are no more registered hosts, so remove the registration

  // get IInternetSession
  CComPtr<IInternetSession> pInternetSession;
  IF_FAILED_RET(CoInternetGetSession(0, &pInternetSession, 0));

  // unregister
  pInternetSession->UnregisterNameSpace(pClassFactory,
      lpszScheme);

  // and remove from map
  m_ClassFactories.RemoveKey(lpszScheme);

  return S_OK;
}

