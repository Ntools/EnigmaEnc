/***************************************************************************
 * Copyright 1996-2005 Nobby Noboru Hirano
 * All rights reserved
 *
 * Warning !!!
 * Distribute unfreely,
 * This program sources can be used authorized user only !!
 * exclude executable files, Under working following OS
 *
 *                          hp-ux   hp9000 Series 700
 *                          Linux   ELF 1.2.13 or later
 *                          Microsoft Windows95 Windows NT
 *                Will be
 *                          Solaris 2  SUN OS 4.x
 *
 *
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Nobby Noboru Hirano <nobby@aiware.c>
 ***************************************************************************/

#define	__ENIG_ENC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
# include <sys/file.h>
# include <sys/time.h>
# include <sys/wait.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
 
# include <resolv.h>

# include "enge.h"
# include <nsock.h>

# include <unistd.h>
#else
# include <winsock.h>
# include "enge.h"
# include "nsock.h"
#endif


#ifdef __WIN32__
# define	Get8_Sock(s,b)	recv(s,b,1,0)
# define	Put8_Sock(s,b)	send(s,b,1,0)
#else
# define	Get8_Sock(s,b)	read(s,b,1)
# define	Put8_Sock(s,b)	write(s,b,1)
#endif

int encryption_flag = 0;

extern unsigned char **eng_gear;
extern unsigned char eng_gear_sel;

#ifdef PLFB
static char *plf[] = {
 /*  0     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15 */
	NULL, "a" , NULL, NULL, NULL, NULL, "^y", NULL, "b" , NULL, "8", "*" , NULL, "f" , NULL, "\';B",
	"A" , NULL, NULL, "vC", "\\", ";H", "Lb", NULL, NULL, NULL, NULL, NULL, "K+", NULL, "-j", "[1",
	NULL, NULL, "2" , NULL, NULL, "3" , NULL, "7" , "c" , "4" , NULL, "nn", NULL, NULL, NULL, NULL,
	"~d", NULL, NULL, "k" , NULL, NULL, NULL, "Q2", NULL, NULL, "X" , NULL, NULL, "5", "@" , "gal",
	NULL, "P" , NULL, NULL, "|" , NULL, "ol", NULL, NULL, "hi", NULL, NULL, "%" , NULL, NULL, "t#!~",
	"&" , NULL, "B", "\"", NULL, NULL, NULL, "9" , NULL, "{" , NULL,  "," , NULL, "6" , NULL, NULL,
	NULL, "Kum", NULL, "Mami" , NULL, NULL, NULL, "a2", "z", NULL, NULL , NULL, NULL, "91", NULL , "^K",
	NULL, NULL, "tako", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "aj" , NULL, "!~", NULL,
	NULL, NULL, NULL, "\"a", NULL, NULL, NULL, "877", NULL, NULL, "Lo", NULL, NULL, NULL, NULL, NULL,
	NULL, "a" , NULL, NULL, NULL, NULL, "^y", NULL, "b" , NULL, "8", "*" , NULL, "f" , NULL, "\';B",
	"A" , NULL, NULL, "vC", "\\", ";H", "Lb", NULL, NULL, NULL, NULL, NULL, "K+", NULL, "-j", "[1",
	NULL, NULL, "2" , NULL, NULL, "3" , NULL, "7" , "c" , "4" , NULL, "nn", NULL, NULL, NULL, NULL,
	"~d", NULL, NULL, "k" , NULL, NULL, NULL, "Q2", NULL, NULL, "X" , NULL, NULL, "5", "@" , "gal",
	NULL, "P" , NULL, NULL, "|" , NULL, "ol", NULL, NULL, "hi", NULL, NULL, "%" , NULL, NULL, "t#!~",
	"&" , NULL, "B", "\"", NULL, NULL, NULL, "9" , NULL, "{" , NULL,  "," , NULL, "6" , NULL, NULL,
	NULL, "Kum", NULL, "Asat" , NULL, NULL, NULL, "c2", NULL, NULL, NULL , NULL, NULL, "91", NULL , "^K",
	NULL, NULL, NULL, "kana", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "aj" , NULL, "!~", NULL,
	NULL, NULL, NULL, "\"a", NULL, NULL, NULL, "877", NULL, NULL, "Lo", NULL, NULL, NULL, NULL, NULL,
};
#endif

static unsigned char encbuffer[2048];
static unsigned char decbuffer[2048];


size_t bufencode(unsigned char *pdbuf, unsigned char *buf, size_t n)
{
	size_t i, p;
	unsigned char off;
#ifdef PLFB
	int j;
#endif

	p = 0;
	for(i = 0;i < n;++i) {
		off = buf[i] ;
		pdbuf[p++] = eng_gear[eng_gear_sel++][off];
#ifdef PLFB
		if(plf[off] != NULL) {
			for(j = 0;plf[off][j] != '\0';++j) pdbuf[p++] = plf[off][j];
		}
#endif
	}
	return(p);
}

size_t bufdecode(unsigned char *pdbuf, unsigned char *buf, size_t n)
{
	size_t i, p;
	int j;

	i = 0;
	p = 0;
	for(p = 0;p < n;++p) {
		for(j = 0;j < 256;++j) {
			if( eng_gear[eng_gear_sel][j] == buf[p]) {
				pdbuf[i++] = j;
#ifdef PLFB
				if(plf[j] != NULL) p += strlen(plf[j]);
#endif
				break;
			}
		}
		++eng_gear_sel;
	}
	return(i);
}

int b_encode(SOCKET sock, int c)
{
	unsigned char sbf[8];
	size_t sz = 0;
#ifdef PLFB
	int j;
#endif

	sbf[sz++] = eng_gear[eng_gear_sel++][c];

#ifdef PLFB
	if(plf[c] != NULL) {
		for(j = 0;plf[c][j] != '\0';++j) sbf[sz++] = plf[c][j];
	}
#endif
	return SockWrite(sock, sbf, sz);
}

int b_decode(SOCKET sock)
{
	int j;
	int c, r;
#ifdef PLFB
	unsigned char sbf[8];
#endif

	Get8_Sock(sock, &c);
	for(j = 0;j < 256;++j) {
		if( eng_gear[eng_gear_sel][j] == c) {
			r = j;
#ifdef PLFB
			if(plf[j] != NULL)
				SockRead(sock, sbf, strlen(plf[j]));
#endif
			return r;
		}
	}
	return -1;
}


char *EncGets(SOCKET sock, char *bf, size_t len)
{
	int r;
	char *buf;

	buf = bf;
	if (!encryption_flag) return (SockGets(sock, buf, len));
	while (len) {
		if ((r = b_decode(sock)) < 0) return NULL;
		if (r == '\n') break;
		if (r == '\r') continue;   /* remove all CR then set LF */
		*buf++ = r;
		--len;
	}
	*buf = '\0';
	return (bf);
}

size_t EncRead(SOCKET sock, unsigned char *buf, size_t len)
{
	size_t l = 0;
	size_t r;

	if ((r = SockRead(sock, buf, len)) <= 0 || !encryption_flag)
		return r;

	l = bufdecode(decbuffer, buf, len);
	memcpy(buf, decbuffer, l);

	return (l);
}

size_t EncWrite(SOCKET sock, unsigned char *buf, size_t len)
{
	size_t l = 0;

	if (!encryption_flag) return (SockWrite(sock, buf, len));

	l = bufencode(encbuffer, buf, len);

	SockWrite(sock, encbuffer, l);

	return (1);
}

int EncPrintf(SOCKET sock, char *format, ...)
{
	va_list ap;
	char buf[8192];

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	return EncWrite(sock, buf, strlen(buf));
}
