/****************************************************************************
 * IInternetProtocolExPatch.h : Declaration of IInternetProtocolRootPatch,
 *    IInternetProtocolPatch and IInternetProtocolExPatch
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "URLMonPatchBase.h"
#include "Protocol.h"

namespace LibInetProtocolHook
{

#define IMPLEMENT_ADAPTER_CALL0(_name)  \
    if (getInst().isEnabled()) { \
      GlobalProtocolAdapterMap map; \
      GlobalProtocolAdapterMap::iterator it = map.find(aInstance); \
      if (it != map.end()) { \
        return it->second->_name(); \
      } \
    } \
    return getInst().do_##_name(aInstance); 

#define IMPLEMENT_ADAPTER_CALL(_name, ...)  \
    if (getInst().isEnabled()) { \
      GlobalProtocolAdapterMap map; \
      GlobalProtocolAdapterMap::iterator it = map.find(aInstance); \
      if (it != map.end()) { \
        return it->second->_name(__VA_ARGS__); \
      } \
    } \
    return getInst().do_##_name(aInstance, __VA_ARGS__); 

// get VTABLE from IUnknown
#define UNK_VTABLE(_unk)    (*reinterpret_cast<VTABLE*>((IUnknown*)(_unk)))

/*============================================================================
 * class IInternetProtocolRootPatch
 *  -> IUnknown
 */
class IInternetProtocolRootPatch :
    public IUnknownPatch
{
protected:
  //--------------------------------------------------------------------------
  // vtable indices for IInternetProtocolRoot
  enum TVtableIndex {
    VTI_Start = __super::VTI_NEXT__,  // previous: IUnknown::Release
    VTI_Continue,
    VTI_Abort,
    VTI_Terminate,
    VTI_Suspend,
    VTI_Resume,
    VTI_NEXT__
  };
};

/*============================================================================
 * class IInternetProtocolPatch
 *  -> IInternetProtocolRoot -> IUnknown
 */
class IInternetProtocolPatch :
    public IInternetProtocolRootPatch
{
protected:
  //--------------------------------------------------------------------------
  // vtable indices for IInternetProtocol
  enum TVtableIndex {
    VTI_Read = __super::VTI_NEXT__, // previous: IInternetProtocolRoot::Resume
    VTI_Seek,
    VTI_LockRequest,
    VTI_UnlockRequest,
    VTI_NEXT__
  };
};

/*============================================================================
 * class IInternetProtocolExPatch
 *
 * This class patches IInternetProtocolEx and its inheritance chain down to
 * IInternetProtocolRoot:
 *    IInternetProtocolEx -> IInternetProtocol -> IInternetProtocolRoot
 */
template<class TSchemeTraits> class IInternetProtocolExPatch :
    public IInternetProtocolPatch,
    public URLMonPatchBase< IInternetProtocolExPatch<TSchemeTraits> >
{
public:
  //--------------------------------------------------------------------------
  // static patch method
  static HRESULT patch()
  {
    // load urlmon
    HMODULE module = ::GetModuleHandle(_T("urlmon.dll"));
    ATLASSERT(module);
    if (!module) {
      return AtlHresultFromLastError();
    }

    // get DllGetClassObject export
    typedef HRESULT (WINAPI* DllGetClassObjectFn)(REFCLSID, REFIID, LPVOID*);
    DllGetClassObjectFn DllGetClassObjectF = (DllGetClassObjectFn)::GetProcAddress(module, "DllGetClassObject");
    ATLASSERT(DllGetClassObjectF);
    if (!DllGetClassObjectF) {
      return AtlHresultFromLastError();
    }

    // get class factory for protocol implementation
    CComPtr<IClassFactory> classFactory;
    HRESULT hr = DllGetClassObjectF(TSchemeTraits::getCLSID(), IID_IClassFactory, (void**)&classFactory.p);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
      return hr;
    }

    // create instance for patching:
    CComPtr<IInternetProtocolEx> dummy;
    hr = classFactory->CreateInstance(NULL, IID_IInternetProtocolEx, (void**)&dummy.p);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
      return hr;
    }

    // patch
    return getInst()._patch(UNK_VTABLE(dummy), TSchemeTraits::getScheme());
  }

  //--------------------------------------------------------------------------
  // static restore method
  static HRESULT restore()
  {
    return getInst()._restore();
  }

  //--------------------------------------------------------------------------
  // public default CTOR
  IInternetProtocolExPatch()
      { }

  //--------------------------------------------------------------------------
  // IUnknown implementation
  DECLARE_VTABLE_PATCH(QueryInterface,
      /* [in] */ REFIID riid,
      /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
  {
#ifdef _DEBUG
    LPOLESTR str;
    if (SUCCEEDED(::StringFromIID(riid, &str))) {
      ATLTRACE(_T("IInternetProtocolExPatch::QueryInterface %s\n"), str);
      ::CoTaskMemFree(str);
    }
    else {
      ATLTRACE(_T("IInternetProtocolExPatch::QueryInterface ???\n"));
    }
#endif
    return getInst().do_QueryInterface(aInstance, riid, ppvObject); 
  }

  //--------------------------------------------------------------------------
  // IInternetProtocolEx implementation
  DECLARE_VTABLE_PATCH(StartEx,
      /* [in] */ IUri *pUri,
      /* [in] */ IInternetProtocolSink *pOIProtSink,
      /* [in] */ IInternetBindInfo *pOIBindInfo,
      /* [in] */ DWORD grfPI,
      /* [in] */ HANDLE_PTR dwReserved)
  {
    if (!getInst().isEnabled()) {
      return getInst().do_StartEx(aInstance, pUri, pOIProtSink, pOIBindInfo, grfPI, dwReserved); 
    }
    // create adapter instance if we don't have yet
    GlobalProtocolAdapterMap map;
    GlobalProtocolAdapterMap::iterator it = map.find(aInstance);
    CComPtr<IInternetProtocolEx> handler;
    if (it == map.end()) {
      handler = Protocol<TSchemeTraits>::createInstance(aInstance);
    }
    else {
      handler = it->second;
    }
    return handler->StartEx(pUri, pOIProtSink, pOIBindInfo, grfPI, dwReserved);
  }

  //--------------------------------------------------------------------------
  // IInternetProtocolRoot implementation
  DECLARE_VTABLE_PATCH(Start,
      /* [in] */ LPCWSTR szUrl,
      /* [in] */ IInternetProtocolSink *pOIProtSink,
      /* [in] */ IInternetBindInfo *pOIBindInfo,
      /* [in] */ DWORD grfPI,
      /* [in] */ HANDLE_PTR dwReserved)
  {
    if (!getInst().isEnabled()) {
      return getInst().do_Start(aInstance, szUrl, pOIProtSink, pOIBindInfo, grfPI, dwReserved); 
    }
    // create adapter instance if we don't have yet
    GlobalProtocolAdapterMap map;
    GlobalProtocolAdapterMap::iterator it = map.find(aInstance);
    CComPtr<IInternetProtocolEx> handler;
    if (it == map.end()) {
      handler = Protocol<TSchemeTraits>::createInstance(aInstance);
    }
    else {
      handler = it->second;
    }
    return handler->Start(szUrl, pOIProtSink, pOIBindInfo, grfPI, dwReserved);
  }

  DECLARE_VTABLE_PATCH(Continue, 
      /* [in] */ PROTOCOLDATA *pProtocolData)
      { IMPLEMENT_ADAPTER_CALL(Continue, pProtocolData); }

  DECLARE_VTABLE_PATCH(Abort,
      /* [in] */ HRESULT hrReason,
      /* [in] */ DWORD dwOptions)
      { IMPLEMENT_ADAPTER_CALL(Abort, hrReason, dwOptions); }

  DECLARE_VTABLE_PATCH(Terminate,
      /* [in] */ DWORD dwOptions)
      { IMPLEMENT_ADAPTER_CALL(Terminate, dwOptions); }

  DECLARE_VTABLE_PATCH0(Suspend)
      { IMPLEMENT_ADAPTER_CALL0(Suspend); }

  DECLARE_VTABLE_PATCH0(Resume)
      { IMPLEMENT_ADAPTER_CALL0(Resume); }

  //--------------------------------------------------------------------------
  // IInternetProtocol implementation
  DECLARE_VTABLE_PATCH(Read,
      /* [length_is][size_is][out][in] */ void *pv,
      /* [in] */  ULONG cb,
      /* [out] */ ULONG *pcbRead)
      { IMPLEMENT_ADAPTER_CALL(Read, pv, cb, pcbRead); }

  DECLARE_VTABLE_PATCH(Seek,
      /* [in] */  LARGE_INTEGER dlibMove,
      /* [in] */  DWORD dwOrigin,
      /* [out] */ ULARGE_INTEGER *plibNewPosition)
      { IMPLEMENT_ADAPTER_CALL(Seek, dlibMove, dwOrigin, plibNewPosition); }

  DECLARE_VTABLE_PATCH(LockRequest,
      /* [in] */ DWORD dwOptions)
      { IMPLEMENT_ADAPTER_CALL(LockRequest, dwOptions); }

  DECLARE_VTABLE_PATCH0(UnlockRequest)
      { IMPLEMENT_ADAPTER_CALL0(UnlockRequest) }

protected:
  //--------------------------------------------------------------------------
  // vtable indices for IInternetProtocolEx
  enum TVtableIndex {
    VTI_StartEx = __super::VTI_NEXT__,  // previous: IInternetProtocol::UnlockRequest
    VTI_NEXT__
  };

private:
  BEGIN_PATCH_MAP(IInternetProtocolExPatch<TSchemeTraits>)
    // IUnknown
    PATCH_MAP_ENTRY(QueryInterface)

    // IInternetProtocolRoot
    PATCH_MAP_ENTRY(Start)
    PATCH_MAP_ENTRY(Continue)
    PATCH_MAP_ENTRY(Abort)
    PATCH_MAP_ENTRY(Terminate)
    PATCH_MAP_ENTRY(Suspend)
    PATCH_MAP_ENTRY(Resume)

    // IInternetProtocol
    PATCH_MAP_ENTRY(Read)
    PATCH_MAP_ENTRY(Seek)
    PATCH_MAP_ENTRY(LockRequest)
    PATCH_MAP_ENTRY(UnlockRequest)

    // IInternetProtocolEx
    PATCH_MAP_ENTRY(StartEx)
  END_PATCH_MAP()

  FORBID_COPY_CONSTRUCTOR(IInternetProtocolExPatch);
};

/*============================================================================
 * Types
 */
typedef IInternetProtocolExPatch< HTTP_Traits >
    IInternetProtocolExPatchHTTP;
typedef IInternetProtocolExPatch< HTTP_S_Traits >
    IInternetProtocolExPatchHTTP_S;

#undef IMPLEMENT_ADAPTER_CALL0
#undef IMPLEMENT_ADAPTER_CALL

} // namespace LibInetProtocolHook
