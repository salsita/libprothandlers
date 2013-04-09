#include "StdAfx.h"
#include "TemporaryProtocolResourceHandler.h"
#include "TemporaryProtocolResourceHandlerClassFactory.h"

// CTOR ResourceHandlerHostInfo
ResourceHandlerHostInfo::ResourceHandlerHostInfo() :
  hResourceInstance(NULL)
{
}

/*****************************************************************************
 * class CTemporaryProtocolResourceHandler
 *****************************************************************************/
//---------------------------------------------------------------------------
// the CLSID for this object
// {5902FEC0-9341-47cd-AAD3-A63FD45E01D4}
const GUID CTemporaryProtocolResourceHandler::CLSID =
  {0x5902fec0, 0x9341, 0x47cd, {0xaa, 0xd3, 0xa6, 0x3f, 0xd4, 0x5e, 0x1, 0xd4}};

//---------------------------------------------------------------------------
// FreeResources
void CTemporaryProtocolResourceHandler::FreeResources()
{
  if (m_hGlobalResource)
  {
    FreeResource(m_hGlobalResource);
  }
  m_hrc = NULL;
  m_hGlobalResource = NULL;
  m_lpData = NULL;
  m_dwSize = 0;
  m_CurrentPos = NULL;
}

//---------------------------------------------------------------------------
// FinalConstruct
HRESULT CTemporaryProtocolResourceHandler::FinalConstruct()
{
  m_hrc = NULL;
  m_hGlobalResource = NULL;
  m_lpData = NULL;
  m_CurrentPos = m_dwSize = 0;
  return S_OK;
}

//---------------------------------------------------------------------------
// FinalRelease
void CTemporaryProtocolResourceHandler::FinalRelease()
{
  FreeResources();
}

//---------------------------------------------------------------------------
// InitializeRequest
HRESULT CTemporaryProtocolResourceHandler::InitializeRequest(
  LPCWSTR lpszPath, DWORD & dwSize)
{
  // strip leading '/'
  CStringW sPath(lpszPath+1);

  // And adjust to resource name scheme.
  // In a resource name no slashes or backslashes are allowed. That's why we
  // use '|' as a path separator.
  sPath.Replace(_T('/'), _T('|'));

  m_hrc = FindResource(m_HostInfo.hResourceInstance, sPath, RT_HTML);
  if (NULL == m_hrc)
  {
    return INET_E_OBJECT_NOT_FOUND;
  }

  m_hGlobalResource = LoadResource(m_HostInfo.hResourceInstance, m_hrc);
  if (!m_hGlobalResource)
  {
    return INET_E_OBJECT_NOT_FOUND;
  }

  m_CurrentPos = 0;
  m_dwSize = dwSize = SizeofResource(m_HostInfo.hResourceInstance, m_hrc);
  m_lpData = (LPBYTE)LockResource(m_hGlobalResource);
  return S_OK;
}

//---------------------------------------------------------------------------
// Read
STDMETHODIMP CTemporaryProtocolResourceHandler::Read(
  void *pv, ULONG cb, ULONG *pcbRead)
{
  if(NULL == m_lpData)
  {
    return E_UNEXPECTED;
  }
  size_t sz = m_dwSize;
  if (m_CurrentPos >= sz)
  {
    return S_FALSE;
  }

  sz -= m_CurrentPos;
  if (0 == sz)
  {
    return S_FALSE;
  }

  if (cb > sz)
  {
    cb = (ULONG)sz;
  }
  memcpy(pv, m_lpData + m_CurrentPos, cb);
  m_CurrentPos += cb;
  if (pcbRead)
  {
    *pcbRead = cb;
  }

  return S_OK;
}

//---------------------------------------------------------------------------
// UnlockRequest
STDMETHODIMP CTemporaryProtocolResourceHandler::UnlockRequest()
{
  FreeResources();
  return S_OK;
}

//---------------------------------------------------------------------------
// Seek
STDMETHODIMP CTemporaryProtocolResourceHandler::Seek(
  LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
  if (!m_lpData)
  {
    return E_UNEXPECTED;
  }
  if (dlibMove.HighPart)
  {
    return E_INVALIDARG;
  }

  switch(dwOrigin)
  {
    case STREAM_SEEK_SET:
      {
        size_t newPos = dlibMove.LowPart;
        if (newPos > m_dwSize)
        {
          return E_FAIL; // after EOF
        }
        m_CurrentPos = newPos;
      }
      break;
    case STREAM_SEEK_CUR:
      {
        size_t ofs = dlibMove.LowPart;
        if (ofs & 0x80000000)
        {
          // negative val
          ofs &= 0x7fffffff;
          if (ofs > m_CurrentPos)
          {
            return E_FAIL; // before SOF
          }
          m_CurrentPos -= ofs;
        }
        else
        {
          if ((ofs + m_CurrentPos) > m_dwSize)
          {
            return E_FAIL; // after EOF
          }
          m_CurrentPos += ofs;
        }
      }
      break;
    case STREAM_SEEK_END:
      if (dlibMove.LowPart > m_dwSize)
      {
        return E_FAIL; // before SOF
      }
      m_CurrentPos = m_dwSize - dlibMove.LowPart;
      break;
  }
  if (plibNewPosition)
  {
    plibNewPosition->HighPart = 0;
    plibNewPosition->LowPart = (DWORD)m_CurrentPos;
  }
  return S_OK;
}
