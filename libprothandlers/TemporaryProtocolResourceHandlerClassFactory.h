#pragma once

#include "TemporaryProtocolHandlerClassFactoryT.h"
#include "TemporaryProtocolResourceHandler.h"

/*****************************************************************************
 * class CTemporaryProtocolResourceHandlerClassFactory
 *  Implements IClassFactory.
 *  Creates instances of CTemporaryProtocolFolderHandler.
 *****************************************************************************/
class ATL_NO_VTABLE CTemporaryProtocolResourceHandlerClassFactory :
  public CTemporaryProtocolHandlerClassFactoryT
        <CTemporaryProtocolResourceHandlerClassFactory,
         CTemporaryProtocolResourceHandler,
         ResourceHandlerHostInfo>,
  public CComObjectRootEx<CComSingleThreadModel>
{
public:
  friend class CProtocolHandlerRegistrar;

  //----------------------------------------------------------------------------
  // com stuff
  DECLARE_NO_REGISTRY()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(CTemporaryProtocolResourceHandlerClassFactory)
    COM_INTERFACE_ENTRY(IClassFactory)
  END_COM_MAP()

  HRESULT FinalConstruct();
  void FinalRelease();

public:
  //-------------------------------------------------------------------------
  // called from CTemporaryProtocolHandlerClassFactoryT when creating a new
  // protocol handler instance to allow initialization
  HRESULT InitHandler(CTemporaryProtocolResourceHandler * pHandler);

protected:
  // called from CProtocolHandlerRegistrar
  // adds a host with the host name and a file name of a DLL or EXE file to
  // load the resources from
  HRESULT AddHost(
    LPCWSTR   lpszHost,
    LPCWSTR   lpszFileName);

  //-------------------------------------------------------------------------
  // called from CProtocolHandlerRegistrar
  // adds a host with the host name and an instance handle to load the
  // resources from
  HRESULT AddHost(
    LPCWSTR   lpszHost,
    HINSTANCE hInstResources);

};
