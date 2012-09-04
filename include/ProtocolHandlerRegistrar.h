#pragma once

#include <atlcoll.h>

/*****************************************************************************
 * class CProtocolHandlerRegistrar
 *  Manages permanent and temporary protocol registrations.
 *  Include this file in your code to use pluggable protocol extension.
 *****************************************************************************/
class CProtocolHandlerRegistrar
{
public:
  ~CProtocolHandlerRegistrar(void);

  //-------------------------------------------------------------------------
  // CProtocolHandlerRegistrar is a singleton
  static CProtocolHandlerRegistrar & GetInstance();

  //-------------------------------------------------------------------------
  // registers a temporary file protocol of the form
  // lpszScheme://lpszHost/
  // where lpszFolder points to the root folder of this "server"
  static HRESULT RegisterTemporaryFolderHandler(
    LPCWSTR lpszScheme,
    LPCWSTR lpszHost,
    LPCWSTR lpszFolder);

  //-------------------------------------------------------------------------
  // registers a temporary resource protocol of the form
  // lpszScheme://lpszHost/
  // where lpszResourceFileName points to the file (DLL or exe) holding the
  // resources
  static HRESULT RegisterTemporaryResourceHandler(
    LPCWSTR lpszScheme,
    LPCWSTR lpszHost,
    LPCWSTR lpszResourceFileName);

  //-------------------------------------------------------------------------
  // registers a temporary resource protocol of the form
  // lpszScheme://lpszHost/
  // where hInstResources is a module handle of the file (DLL or exe) holding
  // the resources
  static HRESULT RegisterTemporaryResourceHandler(
    LPCWSTR   lpszScheme,
    LPCWSTR   lpszHost,
    HINSTANCE hInstResources);

  //-------------------------------------------------------------------------
  // unregisters a file protocol previously registered with
  // one of the RegisterTemporaryXXXHandler methods
  static HRESULT UnregisterTemporaryFolderHandler(
    LPCWSTR lpszScheme,
    LPCWSTR lpszHost);

  //-------------------------------------------------------------------------
  // unregisters a resource protocol previously registered with
  // one of the RegisterTemporaryXXXHandler methods
  static HRESULT UnregisterTemporaryResourceHandler(
    LPCWSTR lpszScheme,
    LPCWSTR lpszHost);

  //-------------------------------------------------------------------------
  // adds a URL where the content resides in memory.
  static HRESULT AddURL(
    LPCWSTR lpszURL,
    LPCVOID lpData,
    DWORD dwLength,
    LPCWSTR lpszMimeType = NULL);

private:
  CProtocolHandlerRegistrar(void);

  //-------------------------------------------------------------------------
  // internal registration function, called from
  // RegisterTemporaryFolderHandler and
  // RegisterTemporaryResourceHandler.
  // CF is the class factory class (currently one of
  // CTemporaryProtocolFolderHandlerClassFactory or
  // CTemporaryProtocolResourceHandlerClassFactory)
  // RT is the type of the resource identifyer argument (e.g. a LPCWSTR for
  // folder / filenames, HINSTANCE for resource instance handle)
  template<class CF, class RT> HRESULT RegisterTemporaryHandler(
    LPCWSTR lpszScheme,
    LPCWSTR lpszHost,
    RT      tResourceID);

  //-------------------------------------------------------------------------
  // internal unregistration function, called from
  // UnregisterTemporaryResourceHandler and
  // UnregisterTemporaryFolderHandler.
  template<class CF> HRESULT UnregisterTemporaryHandler(
    LPCWSTR lpszScheme,
    LPCWSTR lpszHost);

  //-------------------------------------------------------------------------
  // non static version of AddURL
  HRESULT InternalAddURL(
    LPCWSTR lpszURL,
    LPCVOID lpData,
    DWORD dwLength,
    LPCWSTR lpszMimeType = NULL);

  // critical section protecting protocols map
  CRITICAL_SECTION      m_CriticalSection;

  // map holding all temporary protocols (their class factories)
  CAtlMap<CStringW, CComPtr<IClassFactory> >  m_ClassFactories;
};
