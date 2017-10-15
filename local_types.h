#ifndef __LOCAL_TYPES_H__
#define __LOCAL_TYPES_H__

/* $Id: local-types.h,v 1.9 2016/05/16 19:53:46 cvsremote Exp $

   local-types.h: local generic types


   Copyright (c) 2005-2011, Yoichi Hariguchi
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       o Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       o Redistributions in binary form must reproduce the above
         copyright notice, this list of conditions and the following
         disclaimer in the documentation and/or other materials provided
         with the distribution.
       o Neither the name of the Yoichi Hariguchi nor the names of its
         contributors may be used to endorse or promote products derived
         from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/multiprecision/cpp_int.hpp>

typedef boost::multiprecision::uint128_t u128;
typedef boost::multiprecision::uint256_t u256;
typedef boost::multiprecision::int128_t  s128;
typedef boost::multiprecision::int256_t  s256;

typedef u128 U128;
typedef u256 U256;
typedef s128 S128;
typedef s256 S256;
#endif /* __cplusplus */

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t  U8;
typedef int64_t  S64;
typedef int32_t  S32;
typedef int16_t  S16;
typedef int8_t   S8;

typedef u32 ipv4a;              /* in the host byte order */
typedef u32 ipv4na;             /* in the network byte order */


enum {
    SUCCESS = 0,
    FAILURE = -1,
};

#ifndef __cplusplus
typedef enum bool_e {
    TRUE  = 1,
    FALSE = 0
} bool, boolean;
#endif /* __cplusplus */

typedef struct revNum_ {
    u16 major;                  /* major revision number */
    u16 minor;                  /* minor revision number */
} revNum;


/*
 * Memory allocation/release signature
 */
enum {
    MEM_MTRIE3L = 0,
    MEM_TBITMAP,
};



#define elementsOf(_array_)       (sizeof((_array_))/sizeof((_array_)[0]))
#define offsetOf(_type_,_member_) offsetof(_type_, _member_)
#define structHeadOf(_type_,_ptr_,_mbr_) \
           ((_type_*)((char*)(_ptr_) - offsetOf(_type_, _mbr_)))

#define LITERAL_TO_STRING(_literal_) #_literal_
#define MK_STR(_literal_) LITERAL_TO_STRING(_literal_)


#if __SIZEOF_POINTER__ == 4
# define getPtr(_type_,_ptr_)      ((_type_*) \
                                   ((((u32)(_ptr_)) & ((~0) << 2))))
# define getPtrTag(_ptr_)          (((u32)(_ptr_)) & 3)
# define setPtrTag(_ptr_,_val_)    do {     \
           (*((u32*)(_ptr_))) |= ((_val_) & 3);    \
         } while (0)
# define writePtrTag(_ptr_,_val_)  do {     \
           (*((u32*)(_ptr_))) = (_val_);    \
         } while (0)
#elif __SIZEOF_POINTER__ == 8
# define getPtr(_type_,_ptr_)      ((_type_*) \
                                   ((((u64)(_ptr_)) & ((~0) << 2))))
# define getPtrTag(_ptr_)          (((u64)(_ptr_)) & 3)
# define setPtrTag(_ptr_,_val_)    do {     \
           (*((u64*)(_ptr_))) |= ((_val_) & 3);    \
         } while (0)
# define writePtrTag(_ptr_,_val_)  do {     \
           (*((u64*)(_ptr_))) = (_val_);    \
         } while (0)
#endif /* #if __SIZEOF_POINTER__ == 4 */

#define panic(_str_)              do {         \
    printf("\nPANIC!! in %s (%s: %d): ",       \
           __FUNCTION__, __FILE__, __LINE__);  \
    printf _str_;                              \
    printf("\n");                              \
    abort();                                   \
} while (0)


/*
 * C++ specific inline functions
 */
#ifdef __cplusplus

template<class C, size_t N>
inline size_t elementsof(C (&array)[N])
{
    return N;
}

#endif /* __cplusplus */



#endif /* __LOCAL_TYPES_H__ */
