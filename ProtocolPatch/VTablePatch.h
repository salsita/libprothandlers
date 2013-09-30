/****************************************************************************
 * VTablePatch.h : Patching virtual function tables.
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

#include "GlobalMap.h"

namespace LibInetProtocolHook
{
/*============================================================================
 * types
 */

// VTABLE type: a pointer to a vtable (array of PROC)
typedef PROC* VTABLE;


// A global map with all patched vtables. We provide this map to have a way
// to check if a certain vtable is already patched.
// Value is an integer, can be used as an ID or simply as a flag.
typedef GlobalMapAccessor< std::unordered_map<VTABLE, int> > GlobalVtableMap;

/*============================================================================
 * class VTablePatch
 *  This class provides a typesafe patch for one single method in virtual
 *  function table.
 *  Template argument is the type of the method being patched.
 *  For IUnknown::QueryInterface it would be:
 *      HRESULT (STDMETHODCALLTYPE *)(IUnknown *, REFIID, void**)
 *  NOTE: The first argument is always the "this" pointer of the current
 *  instance.
 */

template<typename Tfn>
class VTablePatch
{
public:
  VTablePatch() :
    mTargetAddress(NULL), mOriginalFn(NULL)
  { }

  ~VTablePatch() {
    restore();
  }

  // restore: Remove the hook previously set.
  HRESULT restore() {
    if (!mTargetAddress || !mOriginalFn) {
      // we are already restored
      return S_FALSE;
    }
    DWORD dwOldProt = 0;
    if( !VirtualProtect(mTargetAddress, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt) ) {
      return AtlHresultFromLastError();
    }
    (*mTargetAddress) = mOriginalFn;
    VirtualProtect(mTargetAddress, sizeof(PROC), dwOldProt, &dwOldProt);
    mOriginalFn = NULL;
    mTargetAddress = NULL;
    return S_OK;
  }

  // patch: Set the patch and remember original method.
  HRESULT patch(VTABLE aVtable, size_t aMethodIndex, Tfn aFn) {
    if (mTargetAddress || isHooked(aVtable, aMethodIndex, aFn)) {
      // we are already hooked
      return S_FALSE;
    }
    DWORD dwOldProt = 0;
    if( !VirtualProtect(&aVtable[aMethodIndex], sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt) ) {
      return AtlHresultFromLastError();
    }
    mOriginalFn = reinterpret_cast<Tfn>(aVtable[aMethodIndex]);
    aVtable[aMethodIndex] = reinterpret_cast<PROC>(aFn);
    VirtualProtect(&aVtable[aMethodIndex], sizeof(PROC), dwOldProt, &dwOldProt);
    // remember address for restoring
    mTargetAddress = reinterpret_cast<Tfn*>(&aVtable[aMethodIndex]);
    return S_OK;
  }

  // isHooked: Check if we have already a hook on this method.
  static BOOL isHooked(VTABLE aVtable, size_t aMethodIndex, Tfn aFn) {
    return (aVtable[aMethodIndex] == (PROC)aFn);
  }

  // This acts as a (...) operator for calling the original method.
  // So if you have a patch:
  //  VTablePatch<HRESULT (STDMETHODCALLTYPE *)(IUnknown *, REFIID, void**)>
  //     mMyQueryInterface;
  // you can call the original method as:
  //  mMyQueryInterface(aTargetUnknown, aRefIID, aVoidPtrPtr);
  operator Tfn () {
    ATLASSUME(NULL != mOriginalFn);
    return mOriginalFn;
  }

private:
  // address of the vtable entry being replaced
  Tfn * mTargetAddress;
  // original function
  Tfn mOriginalFn;

  FORBID_COPY_CONSTRUCTOR(VTablePatch);
};

/*============================================================================
 * class IUnknownPatch
 *  Enumerates the basic IUnknown methods.
 *  Acts as a base class for COM object patches.
 */
class IUnknownPatch
{
protected:
  //--------------------------------------------------------------------------
  // vtable indices for IUnknown
  enum TVtableIndex {
    VTI_QueryInterface = 0,
    VTI_AddRef,
    VTI_Release,
    VTI_NEXT__
  };
};

} // namespace LibInetProtocolHook
