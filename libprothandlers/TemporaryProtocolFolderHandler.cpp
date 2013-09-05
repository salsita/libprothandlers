#include "StdAfx.h"
#include "TemporaryProtocolFolderHandler.h"
#include "TemporaryProtocolFolderHandlerClassFactory.h"

/*****************************************************************************
 * class CTemporaryProtocolFolderHandler
 *****************************************************************************/
//---------------------------------------------------------------------------
// the CLSID for this object
// {FB686375-14B8-4ff3-B4DA-DC6BA1D54233}
const GUID CTemporaryProtocolFolderHandler::CLSID =
  {0xfb686375, 0x14b8, 0x4ff3, {0xb4, 0xda, 0xdc, 0x6b, 0xa1, 0xd5, 0x42, 0x33}};

//---------------------------------------------------------------------------
// FreeResources
void CTemporaryProtocolFolderHandler::FreeResources()
{
  m_SpecialURLResource.clear();
  m_File.Close();
}

//---------------------------------------------------------------------------
// FinalConstruct
HRESULT CTemporaryProtocolFolderHandler::FinalConstruct()
{
  return S_OK;
}

//---------------------------------------------------------------------------
// FinalRelease
void CTemporaryProtocolFolderHandler::FinalRelease()
{
  FreeResources();
}

//---------------------------------------------------------------------------
// InitializeRequest
HRESULT CTemporaryProtocolFolderHandler::InitializeRequest(
  LPCWSTR lpszPath, DWORD & dwSize)
{
  // compose absolute file name and strip leading '/' from incoming path
  CStringW sFileName(m_HostInfo.folderName + (lpszPath+1));
  sFileName.Replace(_T('/'), _T('\\'));

  // and open the file
  IF_FAILED_RET(m_File.Create(sFileName, GENERIC_READ,
                FILE_SHARE_READ, OPEN_EXISTING));

  // get file size
  ULONGLONG fileLength;
  IF_FAILED_RET(m_File.GetSize(fileLength));

  // only files <4GB supported
  if (fileLength > 0x00000000ffffffff)
  {
    return INET_E_OBJECT_NOT_FOUND;
  }
  dwSize = (DWORD)fileLength;

  return S_OK;
}

//---------------------------------------------------------------------------
// Read
STDMETHODIMP CTemporaryProtocolFolderHandler::Read(
  void *pv, ULONG cb, ULONG *pcbRead)
{
  if (m_SpecialURLResource.mData) {
    // have a special URL
    return m_SpecialURLResource.read(pv, cb, pcbRead);
  }
  DWORD bytesRead = 0;
  HRESULT hr = m_File.Read(pv, cb, bytesRead);
  if (pcbRead)
  {
    (*pcbRead) = bytesRead;
  }
  if (FAILED(hr)) {
    return hr;
  }
  return (0 == bytesRead)
    ? S_FALSE
    : hr;
}

//---------------------------------------------------------------------------
// UnlockRequest
STDMETHODIMP CTemporaryProtocolFolderHandler::UnlockRequest()
{
  FreeResources();
  return S_OK;
}

//---------------------------------------------------------------------------
// Seek
STDMETHODIMP CTemporaryProtocolFolderHandler::Seek(
  LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
  if (m_SpecialURLResource.mData) {
    // have a special URL
    return m_SpecialURLResource.seek(dlibMove, dwOrigin, plibNewPosition);
  }
  // simply forward to file
  IF_FAILED_RET(m_File.Seek(dlibMove.QuadPart, dwOrigin));
  if (plibNewPosition)
  {
    IF_FAILED_RET(m_File.GetPosition(plibNewPosition->QuadPart));
  }
  return S_OK;
}

