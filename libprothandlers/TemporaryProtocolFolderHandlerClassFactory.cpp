#include "StdAfx.h"

#include "TemporaryProtocolFolderHandlerClassFactory.h"

/*****************************************************************************
 * class CTemporaryProtocolFolderHandler
 *****************************************************************************/

//---------------------------------------------------------------------------
// FinalConstruct
HRESULT CTemporaryProtocolFolderHandlerClassFactory::FinalConstruct()
{
  return S_OK;
}

//---------------------------------------------------------------------------
// FinalRelease
void CTemporaryProtocolFolderHandlerClassFactory::FinalRelease()
{
}

//-------------------------------------------------------------------------
// AddHost
HRESULT CTemporaryProtocolFolderHandlerClassFactory::AddHost(
  LPCWSTR lpszHost,
  LPCWSTR lpszFolderName)
{
  CritSectLock lock(m_CriticalSection);

  // check arguments
  if (!wcslen(lpszHost) || !wcslen(lpszFolderName))
  {
    return E_INVALIDARG;
  }

  // lookup host if we have already
  FolderHandlerHostInfo hostInfo;
  if (LookupHostInfo(lpszHost, hostInfo))
  {
    // found
    return S_FALSE;
  }

  hostInfo.hostName = lpszHost;
  hostInfo.folderName = lpszFolderName;

  // append backslash to folder
  PathAddBackslash(hostInfo.folderName.GetBuffer(MAX_PATH));
  hostInfo.folderName.ReleaseBuffer();

  // add to map
  SetHostInfo(lpszHost, hostInfo);

  return S_OK;
}

//---------------------------------------------------------------------------
// InitHandler
HRESULT CTemporaryProtocolFolderHandlerClassFactory::InitHandler(
  CTemporaryProtocolFolderHandler * pHandler)
{
  return pHandler->Init(this);
}

