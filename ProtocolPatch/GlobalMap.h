/****************************************************************************
 * GlobalMap.h : Declaration of a global Map (GlobalMapAccessor and GlobalMap)
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include <unordered_map>

namespace LibInetProtocolHook
{

/*============================================================================
 * template class GlobalMapAccessor
 * This class is used like a std::map, but provides a thread-safe
 * singleton implementation for a map of this exact type.
 *
 * To access the map simply create an instance of GlobalMapAccessor. The CTOR
 * will lock the map, DTOR unlock it.
 * So as long as the instance lives, the global map is locked.
 * DON'T store a GlobalMapAccessor as a class member anywhere!
 * Consecutive locking per thread is supported.
 */
template <class TMap>
    class GlobalMapAccessor
{
public:
  // Expose these types for easy use and for the wrapped functions:
  typedef typename TMap::iterator        iterator;
  typedef typename TMap::const_iterator  const_iterator;
  typedef typename TMap::key_type        key_type;
  typedef typename TMap::mapped_type     mapped_type;
  typedef typename TMap::size_type       size_type;

public:
  // CTOR: Lock mutex.
  GlobalMapAccessor(void)
    { ::EnterCriticalSection(getMutex()); }

  // DTOR: Unlock mutex.
  ~GlobalMapAccessor(void)
    { ::LeaveCriticalSection(getMutex()); }

  // Wrap some TMap methods.
  // Only a few methods are currently exposed, add more if you need them.
  // The underlying TMap should never be exposed directly
  // (e.g. by overloading GlobalMapAccessor::-> operator).

  // clear()
  void clear()
      { getMap().clear(); }

  // operator []
  mapped_type & operator[] (const key_type& aUnkKey)
      { return getMap()[aUnkKey]; }

  // begin()
  iterator begin()
      { return getMap().begin(); }
  // begin() const
  const_iterator begin() const
      { return getMap().begin(); }

  // end()
  iterator end()
      { return getMap().end(); }
  // end() const
  const_iterator end() const
      { return getMap().end(); }

  // find(const key_type&)
  iterator find(const key_type& aUnkKey)
      { return getMap().find(aUnkKey); }
  // find(const key_type&) const
  const_iterator find(const key_type& aUnkKey) const
      { return getMap().find(aUnkKey); }

  // erase(const key_type&)
  size_type erase(const key_type& aUnkKey)
      { return getMap().erase(aUnkKey); }
  // erase(const_iterator)
	iterator erase(const_iterator _Plist)
      { return getMap().erase(_Plist); }

  // size()
  size_type size() const
      { return getMap().size(); }

  // convenience method: exists(const key_type& aUnkKey)
  bool exists(const key_type& aUnkKey) const
  {
    TMap & map = getMap();
    return (map.end() != map.find(aUnkKey));
  }

private:
  // private acessors to mutex and map.
  // To ensure proper locking these methods must only be called from
  // GlobalMapAccessor instances!
  CRITICAL_SECTION * getMutex() const
      { return &GlobalMap::getInst().mMutex; }
  TMap & getMap() const
      { return GlobalMap::getInst().mMap; }

private:
  //--------------------------------------------------------------------------
  // private class GlobalMap: Holds the actual TMap and the mutex.
  // It's never exposed to clients.
  class GlobalMap
  {
  public:
    // static instance
    static GlobalMap & getInst()
    {
      static GlobalMap instance;
      return instance;
    }

    // CTOR: Initialize mutex.
    GlobalMap(void)
        { ::InitializeCriticalSection(&mMutex); }

    // DTOR: Delete mutex.
    ~GlobalMap(void)
    {
      // This indicates a programming error. Map should be empty when it gets
      // destroyed.
      ATLASSERT(mMap.empty() && "GlobalMap should be empty on destruction.");
      ::DeleteCriticalSection(&mMutex);
    }

    // The mutex. We use a critical section, because it is way faster
    // than a "real" mutex. And we really don't need a cross-process lock.
    CRITICAL_SECTION  mMutex;
    // the actual map
    TMap        mMap;
  };

  FORBID_COPY_CONSTRUCTOR(GlobalMapAccessor);
};

} // namespace LibInetProtocolHook
