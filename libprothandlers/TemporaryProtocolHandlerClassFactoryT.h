#pragma once

#include <atlcoll.h>

/*****************************************************************************
 * Interface IProtocolMemoryResource
 *  Adds support for special URLs - URLs that don't exist pysically.
 *  The data for these resources comes directly from a memory buffer.
 *****************************************************************************/
MIDL_INTERFACE("D3C4D0CB-DEC9-4529-8DDD-8F441E76D584")
IProtocolMemoryResource : public IUnknown
{
public:
    STDMETHOD(AddResource)(
        IUri * aURI,
        LPCVOID lpData,
        DWORD dwLength,
        LPCWSTR lpszMimeType) PURE;

    STDMETHOD(GetResource)(
        IUri * aUri,
        URLMemoryResource & aRetBuffer) PURE;

};

/*****************************************************************************
 * class CTemporaryProtocolHandlerClassFactoryT
 *  Implements IClassFactory.
 *  Creates instances of CTemporaryProtocolFolderHandler.
 *****************************************************************************/
template<class T, class H, class HI>
  class ATL_NO_VTABLE CTemporaryProtocolHandlerClassFactoryT :
    public IClassFactory
{
public:
  friend class CProtocolHandlerRegistrar;

  //----------------------------------------------------------------------------
  // CTOR / DTOR
  CTemporaryProtocolHandlerClassFactoryT()
  {
    InitializeCriticalSection(&m_CriticalSection);
  }

  virtual ~CTemporaryProtocolHandlerClassFactoryT()
  {
    DeleteCriticalSection(&m_CriticalSection);
  }

  //-------------------------------------------------------------------------
  // called from handlers to check if this is our scheme
  BOOL CheckScheme(
    BSTR bsScheme)
  {
    return (m_sScheme == bsScheme);
  }

  //-------------------------------------------------------------------------
  // called from handlers to get the resource (host) info for a request
  BOOL GetResourceInfo(
    LPCWSTR lpszHost, HI & hostInfo)
  {
    CritSectLock lock(m_CriticalSection);
    return m_HostInfos.Lookup(lpszHost, hostInfo);
  }

public:
  //----------------------------------------------------------------------------
  // IClassFactory implementation
  STDMETHOD(CreateInstance)(
    IUnknown *pUnkOuter,
    REFIID riid, void **ppvObject)
  {
    if (pUnkOuter)
    {
      // we don't support aggregation
      return CLASS_E_NOAGGREGATION;
    }

    // the handler instance
    CComObject<H> *pHandler = NULL;

    // create the handler instance
    IF_FAILED_RET(
      CComObject<H>::CreateInstance(&pHandler)
    );

    // make lifetime management safe
    CComPtr<IInternetProtocol> pInternetProtocol = pHandler;

    // and init the handler
    IF_FAILED_RET(static_cast<T*>(this)->InitHandler(pHandler));

    // return whatever interface caller wants or E_NOINTERFACE
    return pHandler->QueryInterface(riid, ppvObject);
  }

  STDMETHOD(LockServer)(
    BOOL fLock)
  {
    // this method is not used
    return S_OK;
  }

protected:
  //----------------------------------------------------------------------------
  // called from CProtocolHandlerRegistrar
  // initializes the class factory with the protocol string
  HRESULT Init(
    LPCWSTR lpszScheme)
  {
    if (!wcslen(lpszScheme))
    {
      return E_INVALIDARG;
    }
    m_sScheme = lpszScheme;

    return S_OK;
  }

  //-------------------------------------------------------------------------
  // called from CProtocolHandlerRegistrar
  // removes a host from the internal map
  size_t RemoveHost(
    LPCWSTR lpszHost)
  {
    CritSectLock lock(m_CriticalSection);
    m_HostInfos.RemoveKey(lpszHost);
    return m_HostInfos.GetCount();
  }

  // lookup a host
  BOOL LookupHostInfo(CStringW key, HI & val)
  {
    CritSectLock lock(m_CriticalSection);
    return m_HostInfos.Lookup(key, val);
  }

  // set a host
  void SetHostInfo(CStringW key, HI & val)
  {
    CritSectLock lock(m_CriticalSection);
    m_HostInfos[key] = val;
  }

protected:
  //----------------------------------------------------------------------------
  // protected data members

  // critical section protecting hosts map
  CRITICAL_SECTION      m_CriticalSection;

private:
  //----------------------------------------------------------------------------
  // private data members

  // the scheme. See ProtocolHandlerRegistrar.h
  CStringW              m_sScheme;

  // map for registered hosts: maps a host name to a HostInfo
  CAtlMap<CStringW, HI> m_HostInfos;
};

