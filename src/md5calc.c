/***********************************************************
 Copyright 2004 by 
         Nobby N Hirano,  All Rights Reserved

**
**
** Autoupdate
**
** Change log:
** Version 0.1.1, 10 Nov 2004. 1st made

**
**
***********************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include "enge.h"

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;

char *md5_checkcode(u8 *sa, u8 *ea)
{
	u32 sum = 0;
	char buf[32];

	while(sa <= ea) sum += *sa++;

	sprintf(buf,"Ntools%08lx", sum);
	return (MD5Digest(buf));
}

char *calc_md5_from_fp(FILE *fp, size_t cnt)
{
	int c;
	u32 sum = 0;
	char buf[32];

	while((c = fgetc(fp)) != EOF) {
		sum += c;
		if (cnt > 0) {
			--cnt;
			if (cnt == 0) break;
		}
	}

	sprintf(buf,"Ntools%08lx", sum);
	return (MD5Digest(buf));
}

char *calc_md5_from_file(char *fnam)
{
	char *p;
	FILE *fp;

	if ((fp = fopen(fnam, "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s\n", fnam);
		return NULL;
	}
	p = calc_md5_from_fp(fp, -1);

	fclose(fp);
	return (p);
}
