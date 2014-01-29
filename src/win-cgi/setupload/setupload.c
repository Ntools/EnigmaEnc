/******************************************************************
                                                                                
  Copyright (C) 2003-4 Nobby Noboru Hirano <nobby@aiware.jp>
  All Rights Reserved.
                                                                                
******************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define	USE_ERRMSG

#include "stdcgi.h"

static char *filename = "KTN10000.dat";
static char *pathname = NULL;

static char *html_str1 = "Content-type: text/html\n\n";

void *xmalloc (size_t size)
{
	void *value = malloc (size);

	if (value == NULL) {
		fprintf(stderr,"xmalloc: virtual memory exhausted.\n");
		exit(1);
	}
	return value;
}

static void disp_html(char *fnam, char *msg)
{
	FILE *fp;
	char buf[512], *p;

	if ((fp = fopen(fnam, "r")) == NULL) {
//		fprintf(stderr, "What can I do ??\n");
        puts("<HTML>");
		puts("<HEAD>");
		puts("<TITLE>Error Message</TITLE>");
		puts("<META HTTP-EQUIV=Content-Type CONTENT=\"text/html; charset=UTF-8\">");
		puts("</HEAD>");
		puts("<BODY BGCOLOR=\"#707088\" TEXT=\"#ffbb00\" LINK=\"#ffff00\" ALINK=\"#ff0000\" VLINK=\"#ffcccc\">");
		puts("<P>");
		puts("<FONT SIZE=5 COLOR=\"RED\">");
		puts("Unknown Error.");
		puts("</P>");
		puts("</FORM>");
		puts("<P>");
		puts("<CENTER><HR></CENTER><BR>");
		puts("</P>");
		puts("</BODY>");
		puts("</HTML>");

		return;
	}
	while(fgets(buf, 512, fp) != NULL) {
		if ((p = strchr(buf, '%')) != NULL) {
			if (*(p + 1) == 's') {
				printf(buf, msg);
				continue;
			}
		}
		printf("%s", buf);
	}
	fclose(fp);
}

#if 0
static char *digit_p(char *p, int *n)
{
	*n = 0;
	while(isdigit(*p)) {
		*n *= 10;
		*n += (int)(*p) - '0';
		++p;
	}
	return p;
}

static int error_msg_init(void)
{
	FILE *fp;
	char buf[BUFSIZ], *p;
	int n;

	for(n = 0;n < ERR_NUM_MAX;++n) error_msg[n] = NULL;

	if ((fp = fopen(ERROR_DEF_FILE, "r")) == NULL) {
		fprintf(stderr, "No Error Message File %s\n", ERROR_DEF_FILE);
		return -1;
	}

	while(fgets(buf, BUFSIZ, fp) != NULL) {
		crcut(buf);
		p = sp_pass(buf);
		if (*p == '#' || *p || ';') continue;
		p = digit_p(p, &n);
		if (n >= ERR_NUM_MAX) {
			fprintf(stderr, "Illegal Error Number [%d] %s\n", n, buf);
			continue;
		}
		error_msg[n] = scopy(sp_pass(p));
	}
	fclose(fp);
	return 0;
}

static void free_err_msg(void)
{
	int n;

	for(n = 0;n < ERR_NUM_MAX;++n) {
		if (error_msg[n] == NULL) free(error_msg[n]);
	}
}
#endif

static void error(int n)
{
	disp_html(ERR_SRC_HTML, error_msg[n]);
	exit(0);
}

static int getform(void)
{
#ifdef _DEBUG
	FILE *fpo;
#endif
	char *buffer, *p;
	size_t n, siz, cnt;
	char *prm[2];
	static char buf[BUFSIZ];

	if((p = getenv("REQUEST_METHOD")) == NULL || strcmp(p, "POST"))
		error(UPD2_SYSTEM_ERR);

	siz = (size_t)atoi(getenv("CONTENT_LENGTH"));
	if((buffer = calloc(siz + 1, sizeof(char))) == NULL) {
		error(UPD2_MEMALOC_ERR);
	}
	cnt = (size_t) siz;
	p = buffer;
	siz = fread(buffer, 1, cnt, stdin );

#ifdef _DEBUG
	fpo = fopen("uploadbuf", "wb");
	if (fpo) {
		fwrite(buffer, 1, siz, fpo);
		fclose(fpo);
	}
#endif

	p = prm[0] = buffer;
	n = 1;
	while (1) {
		if(!strncmp(prm[n-1], "pathname=", 9)) {
			strncpy(buf, sp_pass(&prm[n-1][9]) , BUFSIZ -1);
			pathname = buf;
			com_urlDecode(pathname);
			break;
		}
		if ((p = strchr(p, '&')) != NULL) {
			prm[n] = p + 1;
			*p = '\0';
		}
		else break;
	}

	free(buffer);
	return ((!pathname)? -1: 0);
}

static int cpfile(char *sfile, char *dfile)
{
	FILE *fpi, *fpo;
	int c;
	DIR *pdir;
#ifdef _DEBUG
	FILE *fp;
#endif

#if 0
	sprintf(buf, "copy \"%s\" %s", sfile, dfile);
	system("copy c:\KTN10000.dat KTN10000.dat");
#endif
#ifdef _DEBUG
	fp = fopen("uploadbuf", "a");
	if (fp) {
		fprintf(fp, "\n\nsrc %s dest %s\n", sfile, dfile);
		fclose(fp);
	}
#endif
	if ((fpi = fopen(sfile, "rb")) == NULL) return -1;
	if ((fpo = fopen(dfile, "wb")) == NULL) {
		fclose(fpi);
		return -1;
	}
	while((c = fgetc(fpi)) != EOF) fputc(c, fpo);
	fclose(fpi);
	fclose(fpo);

	system("extruct >nul:");

	// extruct simple check.
	pdir = opendir("var");
	if(pdir)
		closedir(pdir);
	else{
		disp_html(ERR_SRC_HTML, error_msg[UPD2_ILLEGAL_FILE]);
		exit(0);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int s;

//	error_msg_init();

	printf("%s", html_str1);
	s = getform();
	if (s != 0)
		error(UPD2_ILLEGAL_STATUS);
	else s = cpfile(pathname, filename);
	disp_html(UPLOAD_DONE_HTML, "");
//	free_err_msg();
	exit(s);
}
