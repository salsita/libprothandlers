// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	_WIN32_IE_IE70
#define _RICHEDIT_VER	0x0200

#include <atlbase.h>
#include <atlstr.h>
#include <atlapp.h>

#include <atlcom.h>

#include <wininet.h>
#include "Buffer.h"

#ifndef IF_FAILED_RET
#define IF_FAILED_RET(_hr) \
  do \
  { \
    HRESULT _hr__ = _hr; \
    if (FAILED(_hr__)) \
    { \
      return _hr__; \
    } \
  } \
  while(0)
#endif

class CritSectLock
{
public:
  CritSectLock(CRITICAL_SECTION & crit) : m_crit(crit)
  {
    EnterCriticalSection(&m_crit);
  }

  ~CritSectLock()
  {
    LeaveCriticalSection(&m_crit);
  }
private:
  CRITICAL_SECTION & m_crit;
};
