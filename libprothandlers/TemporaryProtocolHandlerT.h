#pragma once

#include <atlfile.h>

/*****************************************************************************
 * class CTemporaryProtocolHandlerT
 *  Baseclass for protocol handlers.
 *
 * T is the implementing class, CF the class factory, HI the host info
 * structure the implementing class uses
 *****************************************************************************/
template<class T, class CF, class HI> class CTemporaryProtocolHandlerT :
  public IInternetProtocol,
  public IInternetProtocolInfo
{
public:
  //----------------------------------------------------------------------------
  // CTOR / DTOR
  CTemporaryProtocolHandlerT() : m_pFactory(NULL)
  {
  }

  virtual ~CTemporaryProtocolHandlerT()
  {
    // !!! raw pointer !!!
    if (m_pFactory)
    {
      m_pFactory->Release();
      m_pFactory = NULL;
    }
  }

  //-------------------------------------------------------------------------
  // IInternetProtocol implementation

  // These two methods at least have to be implemented in the derived class
  STDMETHOD(Read)(
    void *pv,
    ULONG cb,
    ULONG *pcbRead) = 0;

  STDMETHOD(Seek)(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER *plibNewPosition) = 0;

  // Default implementations
  STDMETHOD(LockRequest)(
    DWORD dwOptions)
  {
    return S_OK;
  }

  STDMETHOD(UnlockRequest)()
  {
    return S_OK;
  }

  //-------------------------------------------------------------------------
  // IInternetProtocolRoot implementation
  STDMETHOD(Start)(
    LPCWSTR szUrl,
    IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo,
    DWORD grfPI,
    HANDLE_PTR dwReserved)

  {
    if (!m_pFactory)
    {
      return E_UNEXPECTED;
    }

    // parse our URL
    CComPtr<IUri> pURI;
    IF_FAILED_RET(::CreateUri(szUrl, Uri_CREATE_CANONICALIZE, 0, &pURI));

    CComBSTR bs;

    // get and check scheme
    IF_FAILED_RET(pURI->GetSchemeName(&bs));
    if (!m_pFactory->CheckScheme(bs))
    {
      return INET_E_INVALID_URL;  // not our protocol, don't translate
    }

    // get and check host name
    bs.Empty();
    IF_FAILED_RET(pURI->GetHost(&bs));

    // prepare request: check and get host info
    if (!m_pFactory->GetResourceInfo(bs, m_HostInfo))
    {
      return INET_E_INVALID_URL;  // not our host, don't translate
    }

    // get path
    CStringW sPath, sExtension;
    bs.Empty();
    IF_FAILED_RET(pURI->GetPath(&bs));
    sPath = bs;

    // get extension
    bs.Empty();
    IF_FAILED_RET(pURI->GetExtension(&bs));
    sExtension = bs;

    // unescape path
    DWORD dw = INTERNET_MAX_URL_LENGTH;
    UrlUnescape(sPath.GetBuffer(dw), 0, &dw, URL_UNESCAPE_INPLACE);
    sPath.ReleaseBuffer();

    // get mimetype from extension
    CStringW sMime(_T("application/octet-stream"));
    GetMimeType(sExtension, sMime);

    // let derived class init the request
    DWORD sz = 0;
    IF_FAILED_RET(static_cast<T*>(this)->InitializeRequest(sPath, sz));

    // serve the request to pOIProtSink
    pOIProtSink->ReportProgress(BINDSTATUS_FINDINGRESOURCE, L"Found");
    pOIProtSink->ReportProgress(BINDSTATUS_CONNECTING, L"Connecting");
    pOIProtSink->ReportProgress(BINDSTATUS_SENDINGREQUEST, L"Sending");
    pOIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, sMime);
    pOIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION, 0, sz);
    pOIProtSink->ReportData(BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE, sz, sz);

    pOIProtSink->ReportResult(S_OK, 0, NULL);
    return S_OK;
  }


  STDMETHOD(Continue)(
    PROTOCOLDATA *pProtocolData)
  {
    return S_OK;
  }

  STDMETHOD(Abort)(
    HRESULT hrReason,
    DWORD dwOptions)
  {
    return S_OK;
  }

  STDMETHOD(Terminate)(
    DWORD dwOptions)
  {
    return S_OK;
  }

  STDMETHOD(Suspend)()
  {
    return S_OK;
  }

  STDMETHOD(Resume)()
  {
    return S_OK;
  }

  //-------------------------------------------------------------------------
  // IInternetProtocolInfo implementation
  STDMETHOD(ParseUrl)(
    LPCWSTR pwzUrl,
    PARSEACTION ParseAction,
    DWORD dwParseFlags,
    LPWSTR pwzResult,
    DWORD cchResult,
    DWORD *pcchResult,
    DWORD dwReserved)
  {
    if (!m_pFactory)
    {
      return E_UNEXPECTED;
    }
    switch(ParseAction)
    {
      case PARSE_SECURITY_URL:
      case PARSE_SECURITY_DOMAIN:
        {
          // Security manager wants to know from us. Give answers.
          // The security manager looks at the protocol and domain parts to
          // determine the security to apply here (for preventing XSS etc). It
          // expects ParseURL to return a string: 'scheme:host'.
          // http://msdn.microsoft.com/en-us/library/ms775138%28v=vs.85%29.aspx
          // parse our URL
          CComPtr<IUri> pURI;
          IF_FAILED_RET(::CreateUri(pwzUrl, Uri_CREATE_CANONICALIZE, 0, &pURI));

          CComBSTR bsScheme, bsHost;

          // get and check scheme
          IF_FAILED_RET(pURI->GetSchemeName(&bsScheme));
          if (!m_pFactory->CheckScheme(bsScheme))
          {
            return INET_E_DEFAULT_ACTION;  // not our protocol
          }

          // get and check host name
          IF_FAILED_RET(pURI->GetHost(&bsHost));
          HI hostInfo;
          if (!m_pFactory->GetResourceInfo(bsHost, hostInfo))
          {
            return INET_E_DEFAULT_ACTION;  // not our host
          }

          CStringW sMyURL;
          sMyURL.Format(_T("%s:%s"), bsScheme, bsHost);
          DWORD len = sMyURL.GetLength();
          if (cchResult < (len+1))
          {
            // not enough room to swing a cat here..
            return S_FALSE;
          }
          wcscpy_s(pwzResult, cchResult, sMyURL.GetBuffer(len+1));
          if (pcchResult)
          {
            *pcchResult = len;
          }
          // return string "protocol:host" to security manager
          return S_OK;
        }
        break;
    }
    return INET_E_DEFAULT_ACTION;
  }

  STDMETHOD(CombineUrl)(
    LPCWSTR pwzBaseUrl,
    LPCWSTR pwzRelativeUrl,
    DWORD dwCombineFlags,
    LPWSTR pwzResult,
    DWORD cchResult,
    DWORD *pcchResult,
    DWORD dwReserved)
  {
    return INET_E_DEFAULT_ACTION;
  }

  STDMETHOD(CompareUrl)(
    LPCWSTR pwzUrl1,
    LPCWSTR pwzUrl2,
    DWORD dwCompareFlags)
  {
    return INET_E_DEFAULT_ACTION;
  }

  STDMETHOD(QueryInfo)(
    LPCWSTR pwzUrl,
    QUERYOPTION OueryOption,
    DWORD dwQueryFlags,
    LPVOID pBuffer,
    DWORD cbBuffer,
    DWORD *pcbBuf,
    DWORD dwReserved)
  {
    return INET_E_DEFAULT_ACTION;
  }

protected:
  //-------------------------------------------------------------------------
  // called from CTemporaryProtocolResourceHandlerClassFactory
  // initializes the instance with the protocol string, the host name
  // and the root folder
  HRESULT Init(CF * pFactory)
  {
    if (!pFactory)
    {
      return E_INVALIDARG;
    }
    // !!! raw pointer !!!
    m_pFactory = pFactory;
    m_pFactory->AddRef();
    return S_OK;
  }

  //-------------------------------------------------------------------------
  // GetMimeType: looks up a mimetype from a file extension in the registry
  BOOL GetMimeType(LPCWSTR lpszExtension, CStringW &sMimeType)
  {
    CRegKey regKey;
    LONG res = regKey.Open(HKEY_CLASSES_ROOT, lpszExtension, KEY_READ);
    if (ERROR_SUCCESS != res)
    {
      return FALSE;
    }
    CStringW s;
    ULONG nChars = 1000;
    res = regKey.QueryStringValue(_T("Content Type"), s.GetBuffer(nChars), &nChars);
    s.ReleaseBuffer();
    if (ERROR_SUCCESS != res)
    {
      return FALSE;
    }
    sMimeType = s;
    return TRUE;
  }

protected:
  // Our classfactory. A safepointer would fail to compile, it does not
  // know how to cast this to IUnknown*. So we use a raw pointer.
  CF * m_pFactory;

  // current HostInfo
  HI  m_HostInfo;

};
