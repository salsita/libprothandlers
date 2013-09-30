/****************************************************************************
 * Protocol.h : Declaration of Protocol
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "IInternetProtocolExPatch.h"

namespace LibInetProtocolHook
{

// forward
template<class TSchemeTraits> class IInternetProtocolExPatch;

/*============================================================================
 * class Protocol
 *  Adapter exposing an IInternetProtocolEx interface to the protocol patch.
 *  
 */
template<class TSchemeTraits>
    class ATL_NO_VTABLE Protocol :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IInternetProtocolEx
{
public:
  //--------------------------------------------------------------------------
  // some types
  typedef CComObject< Protocol<TSchemeTraits> >   ProtocolComObject;
  typedef IInternetProtocolExPatch<TSchemeTraits> MyIInternetProtocolExPatch;

  //--------------------------------------------------------------------------
  // static creator
  static CComPtr<IInternetProtocolEx> createInstance(IUnknown * aUnkProtocol)
  {
    ProtocolComObject * adapter = NULL;
    if (FAILED(ProtocolComObject::CreateInstance(&adapter))) {
      return NULL;
    }
    CComPtr<IInternetProtocolEx> owner(adapter);
    if (FAILED(adapter->init(aUnkProtocol))) {
      owner.Release();
    }
    return owner;
  }

  //--------------------------------------------------------------------------
  // CTOR
  Protocol()
      { };

  // -------------------------------------------------------------------------
  // COM map
  BEGIN_COM_MAP(Protocol<TSchemeTraits>)
    COM_INTERFACE_ENTRY(IInternetProtocolRoot)
    COM_INTERFACE_ENTRY(IInternetProtocol)
    COM_INTERFACE_ENTRY(IInternetProtocolEx)
    //COM_INTERFACE_ENTRY(IInternetProtocolInfo)
    //COM_INTERFACE_ENTRY(IInternetPriority)
  END_COM_MAP()

  // -------------------------------------------------------------------------
  // COM standard methods
  HRESULT FinalConstruct()
  {
    return S_OK;
  }

  void FinalRelease()
  {
  }

  // IInternetProtocolRoot
  STDMETHODIMP Start(
      /* [in] */ LPCWSTR szUrl,
      /* [in] */ IInternetProtocolSink *pOIProtSink,
      /* [in] */ IInternetBindInfo *pOIBindInfo,
      /* [in] */ DWORD grfPI,
      /* [in] */ HANDLE_PTR dwReserved)
  {
    mUri.Release();
    HRESULT hr = ::CreateUri(szUrl, Uri_CREATE_CANONICALIZE, 0, &mUri);
    if (SUCCEEDED(hr)) {
      CComBSTR uri;
      mUri->GetAbsoluteUri(&uri);
      ATLTRACE(_T("*** Start request to %s\n"), uri);
    }
    else {
      ATLTRACE(_T("*** Start request to ???\n"));
    }
    return getPatch().do_Start(mUnk, szUrl, pOIProtSink, pOIBindInfo, grfPI, dwReserved);
  }

  STDMETHODIMP Continue(
      /* [in] */ PROTOCOLDATA *pProtocolData)
  {
    return getPatch().do_Continue(mUnk, pProtocolData);
  }

  STDMETHODIMP Abort(
      /* [in] */ HRESULT hrReason,
      /* [in] */ DWORD dwOptions)
  {
    return getPatch().do_Abort(mUnk, hrReason, dwOptions);
  }

  STDMETHODIMP Terminate(
      /* [in] */ DWORD dwOptions)
  {
    ATLASSERT(mUri);
    CComBSTR uri;
    mUri->GetAbsoluteUri(&uri);
    ATLTRACE(_T("*** FINISHED request to %s\n"), uri);
    HRESULT hr = getPatch().do_Terminate(mUnk, dwOptions);
    finalize();
    return hr;
  }

  STDMETHODIMP Suspend()
  {
    return getPatch().do_Suspend(mUnk);
  }

  STDMETHODIMP Resume()
  {
    return getPatch().do_Resume(mUnk);
  }

  // IInternetProtocol
  STDMETHODIMP Read(
      /* [in, out] */ void *pv,
      /* [in] */  ULONG cb,
      /* [out] */ ULONG *pcbRead)
  {
    return getPatch().do_Read(mUnk, pv, cb, pcbRead);
  }

  STDMETHODIMP Seek(
      /* [in] */  LARGE_INTEGER dlibMove,
      /* [in] */  DWORD dwOrigin,
      /* [out] */ ULARGE_INTEGER *plibNewPosition)
  {
    return getPatch().do_Seek(mUnk, dlibMove, dwOrigin, plibNewPosition);
  }

  STDMETHODIMP LockRequest(
      /* [in] */ DWORD dwOptions)
  {
    return getPatch().do_LockRequest(mUnk, dwOptions);
  }

  STDMETHODIMP UnlockRequest()
  {
    HRESULT hr = getPatch().do_UnlockRequest(mUnk);
    finalize();
    return hr;
  }

  // IInternetProtocolEx
  STDMETHODIMP StartEx(
      /* [in] */ IUri *pUri,
      /* [in] */ IInternetProtocolSink *pOIProtSink,
      /* [in] */ IInternetBindInfo *pOIBindInfo,
      /* [in] */ DWORD grfPI,
      /* [in] */ HANDLE_PTR dwReserved)
  {
    mUri = pUri;
    ATLASSERT(mUri);
    CComBSTR uri;
    if (mUri) {
      mUri->GetAbsoluteUri(&uri);
      ATLTRACE(_T("***(%i) StartEX request to %s\n"), ::GetCurrentThreadId(), uri);
    }
    else {
      ATLTRACE(_T("***(%i) StartEX request to ???\n"), ::GetCurrentThreadId());
    }
    ATLASSERT(uri);

    mInternetProtocolSink = pOIProtSink;
    mInternetBindInfo = pOIBindInfo;
    mInternetBindInfoEx = pOIBindInfo;
    mServiceProvider = mInternetProtocolSink;

    // Get current browser. Try global map first
    mBrowser.Release();
    GlobalWebBrowserMap map;
    DWORD threadID = ::GetCurrentThreadId();
    GlobalWebBrowserMap::iterator it = map.find(threadID);
    if (it != map.end()) {
      mBrowser = it->second.mBrowser;
      mNavigationUri = it->second.mUri;
      map.erase(threadID);
    }
    else if (mServiceProvider) {
      // Global map does not have a browser, try service provider
      mServiceProvider->QueryService(SID_SWebBrowserApp, IID_IWebBrowser2, (void**)&mBrowser.p);
    }

    if (mBrowser) {
      ATLTRACE(_T("\thave browser 0x%08x\n"), mBrowser);
    }
    return getPatch().do_StartEx(mUnk, pUri, pOIProtSink, pOIBindInfo, grfPI, dwReserved);
  }

private:
  //--------------------------------------------------------------------------
  // return the MyIInternetProtocolExPatch for this scheme
  static MyIInternetProtocolExPatch & getPatch()
      { return MyIInternetProtocolExPatch::getInst(); }

  //--------------------------------------------------------------------------
  // init method
  HRESULT init(IUnknown * aOriginalInstance)
  {
    // Query all interfaces we need.
    mUnk = aOriginalInstance;

    // add us to global map
    GlobalProtocolAdapterMap map;
    map[mUnk] = this;

    // OK
    return S_OK;
  }

  //--------------------------------------------------------------------------
  // finalize method: removes us from global map
  void finalize()
  {
    // remove us from global object map
    GlobalProtocolAdapterMap map;
    map.erase(mUnk);
  }

  // our associated IUnknown
  CComPtr<IUnknown> mUnk;
  CComPtr<IUri> mNavigationUri;
  CComPtr<IWebBrowser2> mBrowser;
  CComPtr<IInternetProtocolSink> mInternetProtocolSink;
  CComPtr<IInternetBindInfo> mInternetBindInfo;
	CComQIPtr<IInternetBindInfoEx> mInternetBindInfoEx;
	CComQIPtr<IServiceProvider> mServiceProvider;


  CComPtr<IUri> mUri;

	//CComPtr<IUriContainer> m_spUriContainer;

  //CComQIPtr<IInternetProtocolInfo> mInternetProtocolInfo;

};

/*============================================================================
 * struct BrowserRecord
 *  A helper class for GlobalWebBrowserMap containing the browser and the
 *  URL the browser is initially navigating to.
 */
struct BrowserRecord
{
  BrowserRecord(IWebBrowser2 * aBrowser, LPCWSTR aUrl) :
      mBrowser(aBrowser)
  {
    ::CreateUri(aUrl, Uri_CREATE_CANONICALIZE, 0, &mUri);
  }
  BrowserRecord(const BrowserRecord & aRecord) :
      mBrowser(aRecord.mBrowser), mUri(aRecord.mUri)
      { }
  BrowserRecord()
      { }
  CComPtr<IWebBrowser2> mBrowser;
  CComPtr<IUri> mUri;
};

/*============================================================================
 * Types
 */

// The global map storing all Protocol instances
typedef GlobalMapAccessor< std::unordered_map<IUnknown *, CComPtr<IInternetProtocolEx> > >
      GlobalProtocolAdapterMap;

// The global map storing browser objects by thread ID
typedef GlobalMapAccessor< std::unordered_map<DWORD, BrowserRecord> >
      GlobalWebBrowserMap;

} // namespace LibInetProtocolHook
