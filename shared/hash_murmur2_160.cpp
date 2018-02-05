// MurmurHash2_160, by Simon Hardy-Francis
// based upon...
// MurmurHash2, by Austin Appleby
// Note - This code makes a few assumptions about how your machine behaves -
// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
// And it has a few limitations -
// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

#include "stdafx.h"

#include "hash_murmur2_160.h"

void MurmurHash2_160
(
   const void*    pBuf,
   DWORD          dwSize,
   DWORD          dwSeed,
   DWORD          hash32[5]
)
{
   if (!dwSize || !pBuf)
   {
      // Error !
      ASSERT(0);
      return;
   }

   if (IsBadReadPtr(pBuf,dwSize))
   {
      // Error !
      ASSERT(0);
      return;
   }

   // 'm' and 'r' are mixing constants generated offline.
   // They're not really 'magic', they just happen to work well.

   const DWORD    m1 = 0x5BD1E995;
   const DWORD    m2 = 0x19D699A5;
   const DWORD    m3 = 0xB11924E1;
   const DWORD    m4 = 0x16118B03;
   const DWORD    m5 = 0x53C93455;

   const int   r = 24;

   // Initialize the hash to a 'random' value

   DWORD h1 = dwSeed ^ dwSize;        // Original
   DWORD h2 = dwSeed ^ dwSize ^ m2;
   DWORD h3 = dwSeed ^ dwSize ^ m3;
   DWORD h4 = dwSeed ^ dwSize ^ m4;
   DWORD h5 = dwSeed ^ dwSize ^ m5;

   // Mix 4 bytes at a time into the hash

   const BYTE*    data = (const BYTE*)pBuf;
      
   while(dwSize >= 4)
   {  
      DWORD    k1 = *(DWORD*)data;

      k1 *= m1;
      k1 ^= k1 >> r;
      k1 *= m1;

      h1 *= m1;
      h1 ^= k1;
      h1 ^= h2;

      h2 *= m2;
      h2 ^= k1;
      h2 ^= h3;

      h3 *= m3;
      h3 ^= k1;
      h3 ^= h4;

      h4 *= m4;
      h4 ^= k1;
      h4 ^= h5;

      h5 *= m5;
      h5 ^= k1;
      h5 ^= h1;

      data   += 4;
      dwSize -= 4;
   }

   // Handle the last few bytes of the input array

   switch (dwSize)
   {
      case 3:
      {
         h1 ^= data[2] << 16;
         h2 ^= data[2] << 16;
         h3 ^= data[2] << 16;
         h4 ^= data[2] << 16;
         h5 ^= data[2] << 16;
      }
      case 2:
      {
         h1 ^= data[1] << 8;
         h2 ^= data[1] << 8;
         h3 ^= data[1] << 8;
         h4 ^= data[1] << 8;
         h5 ^= data[1] << 8;
      }
      case 1:
      {
         h1 ^= data[0];
         h2 ^= data[0];
         h3 ^= data[0];
         h4 ^= data[0];
         h5 ^= data[0];

         h1 *= m1;
         h2 *= m2;
         h3 *= m3;
         h4 *= m4;
         h5 *= m5;
      }
   }

   // Do a few final mixes of the hash to ensure the last few
   // bytes are well-incorporated.

   h1 ^= h1 >> 13;
   h1 *= m1;
   h1 ^= h1 >> 15;

   h2 ^= h2 >> 13;
   h2 *= m2;
   h2 ^= h2 >> 15;

   h3 ^= h3 >> 13;
   h3 *= m3;
   h3 ^= h3 >> 15;

   h4 ^= h4 >> 13;
   h4 *= m4;
   h4 ^= h4 >> 15;

   h5 ^= h5 >> 13;
   h5 *= m5;
   h5 ^= h5 >> 15;

   hash32[0] = h1 ^ h2 ^ h3 ^ h4 ^ h5;
   hash32[1] = h2 ^ h1;
   hash32[2] = h3 ^ h1;
   hash32[3] = h4 ^ h1;
   hash32[4] = h5 ^ h1;
}
