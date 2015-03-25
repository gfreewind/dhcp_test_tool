#ifndef _MY_BASETYPE_H_
#define _MY_BASETYPE_H_

typedef signed   char       B_S8;    /* [-127, 127] */
typedef unsigned char       B_U8;    /* [0   , 255] */

typedef          short int  B_S16;   /* [-32767 , 32767] */
typedef unsigned short int  B_U16;   /* [0      , 65535] */

typedef          long  int  B_S32;   /* [-2147483647 , 2147483647] */
typedef unsigned long  int  B_U32;   /* [0           , 4294967295] */

typedef long long           B_S64;
typedef unsigned long long  B_U64;

typedef void                B_VOID;
typedef int                 B_BOOL;

typedef B_S8  B_CHAR;
typedef B_U16 B_WCHAR;


#ifdef UNICODE
    typedef B_WCHAR B_TCHAR;
#else
    typedef B_CHAR  B_TCHAR;
#endif

typedef B_VOID*  B_HANDLE;

#define B_TRUE  1
#define B_FALSE 0

#define NULL 0




#endif
