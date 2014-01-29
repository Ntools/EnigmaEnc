/***************************************************************************
 * Copyright 1998-2005 Nobby Noboru Hirano
 * All rights reserved
 *
 * Executable files, Under working following OS
 *
 *                          hp-ux   hp9000 Series 700
 *                          Linux   i386 kernel 2.0.36 or Later
 *                          FreeBSD 2.2.7
 *
 *
 *
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Nobby Noboru Hirano <nobby@ntools.net>
 ***************************************************************************/

#ifndef	ENGE

#ifndef FALSE
# define	FALSE	0
# define	TRUE	(!FALSE)
#endif

#ifdef __cplusplus
#   define EXTERN extern "C"
#else
#   define EXTERN extern
#endif

#ifndef __WIN32__
#   if defined(_WIN32) || defined(WIN32)
#       define __WIN32__
#   endif
#endif

#ifdef __WIN32__
#  ifndef STRICT
#      define STRICT
#  endif
#  ifndef USE_PROTOTYPE
#      define USE_PROTOTYPE 1
#  endif
#  ifndef HAS_STDARG
#      define HAS_STDARG 1
#  endif
#  ifndef USE_PROTOTYPE
#      define USE_PROTOTYPE 1
#  endif
#  ifndef USE_TCLALLOC
#      define USE_TCLALLOC 1
#  endif
#  ifndef STRINGIFY
#      define STRINGIFY(x)         STRINGIFY1(x)
#      define STRINGIFY1(x)        #x
#  endif
#  include <winsock.h>
#  ifdef DLL
#   ifndef DllExport
#    define DllExport	__declspec( dllexport )
#    define DllDef	__declspec( dllimport )
#   endif
#  else
#   ifndef DllExport
#    define DllExport extern
#    define DllDef extern
#   endif
#  endif
#else
#  ifndef DllExport
#   define DllExport extern
#   define DllDef extern
#  endif
#endif /* __WIN32__ */


#ifndef  INADDR_NONE
# ifdef   INADDR_BROADCAST
#  define  INADDR_NONE    INADDR_BROADCAST
# else
#  define  INADDR_NONE    -1
# endif
#endif

#ifndef ON
# define    ON          1
# define    OFF         0
#endif
                                                                                
#ifndef __WIN32__
typedef int SOCKET;
#define	closesocket	close
#endif

#define	Enigma_PORT		18765

extern void crcut(char *s);
extern char *sp_pass(char *p);
extern void show_version(char *);
extern char *calc_md5_from_fp(FILE *, size_t);
extern char *calc_md5_from_file(char *);
extern char *MD5Digest(char *);

#ifndef __ENIG_ENC
extern int bufencode(unsigned char *, unsigned char *, size_t);
extern int bufdecode(unsigned char *, unsigned char *, size_t);
extern int b_decode(SOCKET);
extern int b_encode(SOCKET, int);

extern int encryption_flag;

#define	EncGetc	b_decode
#define	EncPutc	b_encode

extern size_t EncRead(SOCKET, unsigned char *, size_t);
extern size_t EncWrite(SOCKET, unsigned char *, size_t);
extern char *EncGets(SOCKET, char *, size_t);
extern size_t EncPrintf(SOCKET, char *, ...);
#endif

#ifndef __WIN32__
extern void strlwr(char *);
#endif

#define	ENGE

#endif
