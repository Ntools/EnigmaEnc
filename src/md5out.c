/***********************************************************
 Copyright 2004 by 
         Nobby N Hirano, All Rights Reserved

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
#include <unistd.h>

#include "mmap.h"

int main(int argc, char *argv[])
{
	int c, s = 0;
	char *p;
	int wn = 0;

	while ((c = getopt (argc, argv, "nt")) != EOF)
	switch (c) {
	  case 'n':
		wn = 1;
		break;
	  case 't':
		break;
	  default:
		break;
	}

	while (optind < argc) {
		p = calc_md5_from_file(argv[optind]);
		if (p != NULL) {
#ifdef WIN32
			if (wn) printf("%s  %s ", p, argv[optind]);
			else printf("%s ", p);
#else
			if (wn) printf("%s  %s\n", p, argv[optind]);
			else printf("%s\n", p);
#endif
		}
		else s = 1;
		++optind;
	}

	exit(s);
}
