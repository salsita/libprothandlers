#include "StdAfx.h"
#include "Buffer.h"

/*****************************************************************************
 * class SeekableBuffer
 *****************************************************************************/

//---------------------------------------------------------------------------
// read
HRESULT SeekableBuffer::read(
  void *pv,
  ULONG cb,
  ULONG *pcbRead)
{
  if(NULL == mData)
  {
    return E_UNEXPECTED;
  }
  ULONG sz = mLength;
  if (mPos >= sz)
  {
    return S_FALSE;
  }

  sz -= mPos;
  if (0 == sz)
  {
    return S_FALSE;
  }

  if (cb > sz)
  {
    cb = (ULONG)sz;
  }
  memcpy(pv, mData + mPos, cb);
  mPos += cb;
  if (pcbRead)
  {
    *pcbRead = cb;
  }

  return S_OK;
}

//---------------------------------------------------------------------------
// seek
HRESULT SeekableBuffer::seek(
  LARGE_INTEGER dlibMove,
  DWORD dwOrigin,
  ULARGE_INTEGER *plibNewPosition)
{
  if (!mData)
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
        ULONG newPos = dlibMove.LowPart;
        if (newPos > mLength)
        {
          return E_FAIL; // after EOF
        }
        mPos = newPos;
      }
      break;
    case STREAM_SEEK_CUR:
      {
        ULONG ofs = dlibMove.LowPart;
        if (ofs & 0x80000000)
        {
          // negative val
          ofs &= 0x7fffffff;
          if (ofs > mPos)
          {
            return E_FAIL; // before SOF
          }
          mPos -= ofs;
        }
        else
        {
          if ((ofs + mPos) > mLength)
          {
            return E_FAIL; // after EOF
          }
          mPos += ofs;
        }
      }
      break;
    case STREAM_SEEK_END:
      if (dlibMove.LowPart > mLength)
      {
        return E_FAIL; // before SOF
      }
      mPos = mLength - dlibMove.LowPart;
      break;
  }
  if (plibNewPosition)
  {
    plibNewPosition->HighPart = 0;
    plibNewPosition->LowPart = (DWORD)mPos;
  }
  return S_OK;
}

/*****************************************************************************
 * class URLMemoryResource
 *****************************************************************************/

//---------------------------------------------------------------------------
// DTOR
URLMemoryResource::~URLMemoryResource() {
  clear();
}

//---------------------------------------------------------------------------
// clear
void URLMemoryResource::clear() {
  if (mData) {
    delete [] mData;
  }
  mData = NULL;
  mLength = 0;
  mimeType.Empty();
}

//---------------------------------------------------------------------------
// copyFrom
void URLMemoryResource::copyFrom(URLMemoryResource & aOther) {
  setData(aOther.mData, aOther.mLength, aOther.mimeType);
}

//---------------------------------------------------------------------------
// setData
void URLMemoryResource::setData(LPCVOID aDataBuffer, DWORD aLength, LPCWSTR aMimeType) {
  clear();
  mData = new BYTE[aLength];
  mLength = aLength;
  memcpy(mData, aDataBuffer, aLength);
  mimeType = aMimeType;
}

