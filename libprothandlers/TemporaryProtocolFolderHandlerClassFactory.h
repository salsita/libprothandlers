#pragma once

#include "TemporaryProtocolHandlerClassFactoryT.h"
#include "TemporaryProtocolFolderHandler.h"

/*****************************************************************************
 * class CTemporaryProtocolFolderHandlerClassFactory
 *  Implements IClassFactory.
 *  Creates instances of CTemporaryProtocolFolderHandler.
 *****************************************************************************/
class ATL_NO_VTABLE CTemporaryProtocolFolderHandlerClassFactory :
  public CTemporaryProtocolHandlerClassFactoryT
        <CTemporaryProtocolFolderHandlerClassFactory,
         CTemporaryProtocolFolderHandler,
          FolderHandlerHostInfo>,
  public CComObjectRootEx<CComSingleThreadModel>
{
public:
  friend class CProtocolHandlerRegistrar;

  //----------------------------------------------------------------------------
  // com stuff
  DECLARE_NO_REGISTRY()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(CTemporaryProtocolFolderHandlerClassFactory)
    COM_INTERFACE_ENTRY(IClassFactory)
  END_COM_MAP()

  HRESULT FinalConstruct();
  void FinalRelease();

public:
  //-------------------------------------------------------------------------
  // called from CTemporaryProtocolHandlerClassFactoryT when creating a new
  // protocol handler instance to allow initialization
  HRESULT InitHandler(CTemporaryProtocolFolderHandler * pHandler);

protected:
  // called from CProtocolHandlerRegistrar
  // adds a host with the host name and a folder name to load the resources
  // from
  HRESULT AddHost(
    LPCWSTR   lpszHost,
    LPCWSTR   lpszFolderName);

};
