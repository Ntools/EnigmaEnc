/***********************************************************
 Copyright 2004-2005 by 
         Nobby N Hirano, All Rights Reserved
***********************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef WIN32
#include <libgen.h>
#else
#endif 

char *sp_pass(char *p)
{
	while(*p == ' ' || *p == '\t') ++p;
	return (p);
}

void crcut(char *s)
{
	char *p;

	if ((p = strchr(s, 0x0a)) != NULL) *p = '\0';
	if ((p = strchr(s, 0x0d)) != NULL) *p = '\0';
}

#ifndef WIN32
void strlwr(char *p)
{
	while(*p) {
		if(*p >= 'A' && *p <= 'Z') *p += ' ';
		++p;
	}
}
#endif

void show_version(char *nam)
{
#ifndef WIN32
	printf("%s version " AI_VERSION " compiled %s %s\n", basename(nam), __DATE__, __TIME__);
#else
	printf("%s version 0.9.1 compiled %s %s\n", nam, __DATE__, __TIME__);
#endif
}

void com_urlDecode(unsigned char *src)
{
	int	i,j;
	size_t	length;
	unsigned char	dst[BUFSIZ];
	unsigned char	calc;

	length = strlen(src);

	for (i = 0, j = 0;i < length ;i++, j++ ) {
		if ( src[i] == '+' ) dst[j] = ' ';
		else if ( src[i] == '%' ){
			i++ ;
			if ( src[i] >= 'A') calc = src[i] - 'A' + 10;
			else calc = src[i] - '0';

			calc = calc << 4;
			i++ ;
			if ( src[i] >= 'A') calc += src[i] - 'A' + 10;
			else 	calc += src[i] - '0';

			dst[j] = calc;
		} 
		else {
			dst[j] = src[i];
		}
	}
	dst[j] = '\0';
	strcpy( src, dst );
}
