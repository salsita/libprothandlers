#pragma once

#include <atlfile.h>
#include "TemporaryProtocolHandlerT.h"

//-------------------------------------------------------------------------
// struct FolderHandlerHostInfo : contains the host related data
struct FolderHandlerHostInfo
{
  CStringW  hostName;
  CStringW  folderName;
};

// forwards and typedefs
class CTemporaryProtocolFolderHandlerClassFactory;
class CTemporaryProtocolFolderHandler;
typedef CTemporaryProtocolHandlerT
        <CTemporaryProtocolFolderHandler,
         CTemporaryProtocolFolderHandlerClassFactory,
         FolderHandlerHostInfo> CTemporaryProtocolFolderHandlerBase;

/*****************************************************************************
 * class CTemporaryProtocolFolderHandler
 *  Implements IInternetProtocol and IInternetProtocolInfo.
 *  Handles a file based protocol.
 *  This is the actual protocol handler.
 *****************************************************************************/
class ATL_NO_VTABLE CTemporaryProtocolFolderHandler :
  public CTemporaryProtocolFolderHandlerBase,
  public CComObjectRootEx<CComSingleThreadModel>
{
public:
  friend CTemporaryProtocolFolderHandlerClassFactory;
  friend CTemporaryProtocolFolderHandlerBase;

  //-------------------------------------------------------------------------
  // Our CLSID. Need this for registering the handler.
  static const GUID CLSID;

public:
  //-------------------------------------------------------------------------
  // com stuff
  DECLARE_NO_REGISTRY()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(CTemporaryProtocolFolderHandler)
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
  // called from CTemporaryProtocolHandlerT in Start(..)
  HRESULT InitializeRequest(LPCWSTR lpszPath, DWORD & dwSize);

private:
  //-------------------------------------------------------------------------
  // FreeResources: frees all used resources (closes the file)
  void FreeResources();

  //-------------------------------------------------------------------------
  // private data members

  // the file for our URL
  CAtlFile    m_File;
};
