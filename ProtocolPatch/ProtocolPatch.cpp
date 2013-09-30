/****************************************************************************
 * libinetprotocolhook.cpp : Implementation of libinetprotocolhook
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "stdafx.h"
#include "IInternetProtocolExPatch.h"
#include "ProtocolPatch.h"

namespace LibInetProtocolHook
{

//--------------------------------------------------------------------------
// patchProtokols
//  Patches HTTP and HTTPS protocols and the class factories for them.
HRESULT patchProtokols()
{
  // patch HTTP
  HRESULT hr = IInternetProtocolExPatchHTTP::patch();
  if (FAILED(hr)) {
    return hr;
  }

  // patch HTTPS
  hr = IInternetProtocolExPatchHTTP_S::patch();
  if (FAILED(hr)) {
    return hr;
  }

  // switch on the patches
  IInternetProtocolExPatchHTTP::enable();
  IInternetProtocolExPatchHTTP_S::enable();

  return S_OK;
}

//--------------------------------------------------------------------------
HRESULT unpatchProtokols()
{
  // Unpatch in reversed order. In case any methods are patched twice
  // this will guarantee that the original methods are restored properly.
  // Although it does not appear to happen, better safe than sorry.
  HRESULT hr1 = IInternetProtocolExPatchHTTP_S::restore();
  HRESULT hr2 = IInternetProtocolExPatchHTTP::restore();
  return FAILED(hr2) ? hr2 : hr1;
}

//--------------------------------------------------------------------------
HRESULT setBrowserForRequest(LPDISPATCH aDispatch, VARIANT *aVtUrl)
{
  if (!aVtUrl || (VT_BSTR != aVtUrl->vt)) {
    return E_INVALIDARG;
  }
  CComQIPtr<IWebBrowser2> browser(aDispatch);
  if (!browser) {
    return E_NOINTERFACE;
  }
  GlobalWebBrowserMap map;
  ATLASSERT(browser);
  map[::GetCurrentThreadId()] = BrowserRecord(browser, aVtUrl->bstrVal);
  return S_OK;
}

} // namespace LibInetProtocolHook
