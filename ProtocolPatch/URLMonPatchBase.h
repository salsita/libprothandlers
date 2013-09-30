/****************************************************************************
 * URLMonPatchBase.h : Declaration of URLMonPatchBase
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "VTablePatch.h"
#include "Scheme.h"

namespace LibInetProtocolHook
{

/*============================================================================
 * class URLMonPatchBase
 * Baseclass for patches in urlmon.dll.
 */
template<class TImpl>
    class URLMonPatchBase
{
public:
  //--------------------------------------------------------------------------
  // the one and only instance
  static TImpl & getInst()
  {
    static TImpl sInstance;
    return sInstance;
  }

  //--------------------------------------------------------------------------
  // enable / disable patches for this vtable
  static void enable(BOOL aEnable = TRUE)
      { getInst().mEnabled = aEnable; }

  //--------------------------------------------------------------------------
  // isEnabled
  static BOOL isEnabled()
      { return getInst().mEnabled; }

public:
  //--------------------------------------------------------------------------
  // CTOR
  URLMonPatchBase() : mVTable(NULL), mEnabled(FALSE)
      { };

protected:
  // our vtable
  VTABLE  mVTable;
  // flag: patch enabled or not
  BOOL    mEnabled;
};

/*============================================================================
 * Macros for implementing urlmon patches
 */

// The DECLARE_VTABLE_PATCH??(type, MethodName) macros create:
//  - a type "VFTablePatch_MethodName": VTablePatch<function_pointer_type>
//  - an instance of VFTablePatch_MethodName: "do_MethodName"
//  - a patch function: "HRESULT _patch_MethodName(VTABLE aVtable)"
//  - a restore function: "HRESULT _restore_MethodName()"
//  - a static method "MethodName_Hook" of type function_pointer_type
#define DECLARE_VTABLE_PATCH0_(type, name) \
  typedef VTablePatch<type (STDMETHODCALLTYPE *)(IUnknown * aInstance)> VFTablePatch_##name; \
  VFTablePatch_##name do_##name; \
  HRESULT _patch_##name(VTABLE aVtable) { \
    return do_##name.patch(aVtable, VTI_##name, name##_Hook); \
  } \
  HRESULT _restore_##name() { \
    return do_##name.restore(); \
  } \
  static type STDMETHODCALLTYPE name##_Hook(IUnknown * aInstance)

#define DECLARE_VTABLE_PATCH_(type, name, ...) \
  typedef VTablePatch<type (STDMETHODCALLTYPE *)(IUnknown * aInstance, __VA_ARGS__)> name##_t; \
  name##_t do_##name; \
  HRESULT _patch_##name(VTABLE aVtable) { \
    return do_##name.patch(aVtable, VTI_##name, name##_Hook); \
  } \
  HRESULT _restore_##name() { \
    return do_##name.restore(); \
  } \
  static type STDMETHODCALLTYPE name##_Hook(IUnknown * aInstance, __VA_ARGS__)

#define DECLARE_VTABLE_PATCH0(name) DECLARE_VTABLE_PATCH0_(HRESULT, name)

#define DECLARE_VTABLE_PATCH(name, ...) DECLARE_VTABLE_PATCH_(HRESULT, name, __VA_ARGS__)

// BEGIN_PATCH_MAP(cls) macro delcares:
//  - a type "_PatchMapClass": of type cls
//  - a type "_patchFn": for a _patch_MethodName method
//  - a type "_restoreFn": for a _restore_MethodName method
//  - a struct _PatchMapEntry containining a _patchFn / _restoreFn pair
//  - a static method _getPatchMap that returns the patchmap
#define BEGIN_PATCH_MAP(cls) \
	typedef cls _PatchMapClass; \
  typedef HRESULT (_PatchMapClass::*_patchFn)(VTABLE); \
  typedef HRESULT (_PatchMapClass::*_restoreFn)(); \
  struct _PatchMapEntry { \
    _patchFn patch; \
    _restoreFn restore; \
  }; \
  static _PatchMapEntry * _getPatchMap() { \
    static _PatchMapEntry map[] = {

// PATCH_MAP_ENTRY(name) macro adds a _PatchMapEntry containining a
// _patchFn / _restoreFn pair to the patchmap.
#define PATCH_MAP_ENTRY(name) {&_patch_##name, &_restore_##name},

// END_PATCH_MAP() macro delcares:
//  - a terminating _PatchMapEntry containining NULL / NULL
//  - a method _patch(VTABLE aVtable)
//  - a method _restore()
#define END_PATCH_MAP() \
      {NULL, NULL} \
    }; \
    return map; \
  } \
\
  HRESULT _patch(VTABLE aVtable, int aId = 0) { \
    if (mVTable) { \
      return (mVTable == aVtable) ? S_FALSE : E_UNEXPECTED; \
    } \
    GlobalVtableMap map; \
    if (map.exists(aVtable)) { \
      return S_FALSE; \
    } \
    mVTable = aVtable; \
    for (_PatchMapEntry * entry = _getPatchMap(); entry->patch; entry++) { \
      HRESULT hr = ((*this).*entry->patch)(mVTable); \
      ATLASSERT(SUCCEEDED(hr)); \
      if (FAILED(hr)) { \
        return hr; \
      } \
    } \
    map[mVTable] = aId; \
    return S_OK; \
  } \
\
  HRESULT _restore() { \
    if (!mVTable) { \
      return S_FALSE; \
    } \
    for (_PatchMapEntry * entry = _getPatchMap(); entry->restore; entry++) { \
      ATLVERIFY(SUCCEEDED(((*this).*entry->restore)())); \
    } \
    GlobalVtableMap map; \
    map.erase(mVTable); \
    mVTable = NULL; \
    return S_OK; \
  }

} // namespace LibInetProtocolHook
