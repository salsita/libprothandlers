#pragma once

/*****************************************************************************
 * class SeekableBuffer
 *  Buffer implementing read() and seek() the way IStream does.
 *****************************************************************************/
class SeekableBuffer
{
public:
  SeekableBuffer() : mLength(0), mPos(0), mData(NULL) {}
  virtual ~SeekableBuffer() {}

  // IStream::read
  HRESULT read(
    void *pv,
    ULONG cb,
    ULONG *pcbRead);

  // IStream::seek
  HRESULT seek(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER *plibNewPosition);

  ULONG   mLength;
  ULONG   mPos;
  LPBYTE  mData;
};

/*****************************************************************************
 * class URLMemoryResource
 *  Special buffer for handling memory based resources.
 *****************************************************************************/
class URLMemoryResource : public SeekableBuffer
{
public:
  ~URLMemoryResource();

  // clears the buffer, deletes data
  void clear();

  // copy from another buffer
  void copyFrom(URLMemoryResource & aOther);

  // copy data into this object
  void setData(LPCVOID aDataBuffer, DWORD aLength, LPCWSTR aMimeType);

  CStringW mimeType;
};
