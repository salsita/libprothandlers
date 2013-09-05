#pragma once

#include "TemporaryProtocolHandlerT.h"

//-------------------------------------------------------------------------
// struct ResourceHandlerHostInfo : contains the host related data
struct ResourceHandlerHostInfo
{
  ResourceHandlerHostInfo();
  CStringW  hostName;
  HINSTANCE hResourceInstance;
};

// forwards and typedefs
class CTemporaryProtocolResourceHandlerClassFactory;
class CTemporaryProtocolResourceHandler;
typedef CTemporaryProtocolHandlerT
        <CTemporaryProtocolResourceHandler,
         CTemporaryProtocolResourceHandlerClassFactory,
         ResourceHandlerHostInfo> CTemporaryProtocolResourceHandlerBase;

/*****************************************************************************
 * class CTemporaryProtocolResourceHandler
 *  Implements IInternetProtocol and IInternetProtocolInfo.
 *  Handles a file based protocol.
 *  This is the actual protocol handler.
 *****************************************************************************/
class ATL_NO_VTABLE CTemporaryProtocolResourceHandler :
  public SeekableBuffer,
  public CTemporaryProtocolResourceHandlerBase,
  public CComObjectRootEx<CComSingleThreadModel>
{
public:
  friend CTemporaryProtocolResourceHandlerClassFactory;
  friend CTemporaryProtocolResourceHandlerBase;

  //-------------------------------------------------------------------------
  // Our CLSID. Need this for registering the handler.
  static const GUID CLSID;

public:
  //-------------------------------------------------------------------------
  // com stuff
  DECLARE_NO_REGISTRY()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(CTemporaryProtocolResourceHandler)
    COM_INTERFACE_ENTRY(IInternetProtocol)
    COM_INTERFACE_ENTRY(IInternetProtocolRoot)
    COM_INTERFACE_ENTRY(IInternetProtocolInfo)
    COM_INTERFACE_ENTRY2(IUnknown, IInternetProtocol)
  END_COM_MAP()

  HRESULT FinalConstruct();
  void FinalRelease();

public:
  //-------------------------------------------------------------------------
  // IInternetProtocol implementation
  STDMETHOD(Read)(
    void *pv,
    ULONG cb,
    ULONG *pcbRead);

  STDMETHOD(Seek)(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER *plibNewPosition);

  STDMETHOD(UnlockRequest)();

protected:
  //-------------------------------------------------------------------------
  // called from CTemporaryProtocolHandlerT to get the hostname 
  HRESULT InitializeRequest(LPCWSTR lpszPath, DWORD & dwSize);

private:
  //-------------------------------------------------------------------------
  // FreeResources: frees all used resources (closes the file)
  void FreeResources();

  //-------------------------------------------------------------------------
  // private data members

  // Current resource
  HRSRC     m_hrc;

  // HGLOBAL for current resource
  HGLOBAL   m_hGlobalResource;
};
