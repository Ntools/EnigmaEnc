/***********************************************************
 Copyright 2004-2005 by 
         Nobby N Hirano
   All Rights Reserved
**
**
***********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

unsigned long hash(char *str, unsigned m) {
	unsigned long v = 0L;

	for ( ; *str; str++) {
		v = ((v << 7)+ *str);
	}
	return v % m;
}

int main(int argc, char *argv[])
{
	unsigned int md = 60 * 24;
	char buf[128];
	int c;

	while ((c = getopt (argc, argv, "m:")) != EOF)
	switch (c) {
	  case 'm':
		md  = (unsigned int)atoi(optarg);
		break;
	  default:
		break;
	}

	if (optind < argc) {
		while (optind < argc) {
			printf("%lu\n", hash(argv[optind++], md));
		}
	}
	else {
		srand((unsigned int)time(NULL));
		sprintf(buf,"%d", rand());
		printf("%lu\n", hash(buf, md));
	}
	exit (0);
}
