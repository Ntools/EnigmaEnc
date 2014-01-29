/******************************************************************
                                                                                
  Copyright (C) 2003-4 Nobby Noboru Hirano <nobby@aiware.jp>
  All Rights Reserved.
                                                                                
******************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <process.h>

#define	USE_ERRMSG

#include "stdcgi.h"


#define	ERR_HANDLE	"errfile"

#define	PASSWD_FILE	"passwd"

#ifdef UPLOAD
static char *filename = "update2.dat";
#else
static char *filename = "KTN10000.bin";
#endif
#ifdef ENIREC
static char *pathname = "KTN10000.dat";
#else
static char *pathname = NULL;
#endif
static char *hostname = NULL;
static char *username = NULL;
static char *password = NULL;

static char *html_str1 = "Content-type: text/html\n\n";


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
	char *p, *s;
	size_t n, siz, cnt;
	char buffer[512];
	static char pnam[130], hnam[130], unam[130], pswd[130];

	if((p = getenv("REQUEST_METHOD")) == NULL || strcmp(p, "POST"))
		error(UPD2_SYSTEM_ERR);

	siz = (size_t)atoi(getenv("CONTENT_LENGTH"));

	if (siz > 510) {
		puts("<br>Warning buffer size Over fllow, set 511<br>");
		siz = 511;
	}

#ifdef _DEBUG
	fpo = fopen("uploadbuf", "w");
	if (fpo) fprintf(fpo, "write bytes = %d\n", siz);
	if (fpo) fclose(fpo);
#endif
	cnt = (size_t) siz;
	p = buffer;
	siz = fread(buffer, 1, cnt, stdin );
	buffer[siz] = '\0';

#ifdef _DEBUG
	fpo = fopen("uploadbuf", "a");
	if (fpo) {
		fwrite(buffer, 1, siz, fpo);
		fputc('\n', fpo);
	}
	if (fpo) fclose(fpo);
#endif

	p = buffer;
	n = 0;
	while(1) {
		if ((s = strchr(p, '&')) != NULL) *s = '\0';
		if(!strncmp(p, "pathname=", 9)) {
			strncpy(pnam, sp_pass(&p[9]) , 128);
			pnam[128] = '\0';
			com_urlDecode(pnam);
			pathname = pnam;
			crcut(pathname);
#ifdef _DEBUG
			fpo = fopen("uploadbuf", "a");
			if (fpo) fprintf(fpo, "pathname = %s\n", pathname);
			if (fpo) fclose(fpo);
#endif
		}
		else if (!strncmp(p, "hostname=", 9)) {
			strncpy(hnam, sp_pass(&p[9]) , 128);
			hnam[128] = '\0';
			com_urlDecode(hnam);
			hostname = hnam;
			crcut(hostname);
#ifdef _DEBUG
			fpo = fopen("uploadbuf", "a");
			if (fpo) fprintf(fpo, "hostname = %s\n", hostname);
			if (fpo) fclose(fpo);
#endif
		}
		else if (!strncmp(p, "username=", 9)) {
			strncpy(unam, sp_pass(&p[9]) , 128);
			unam[128] = '\0';
			com_urlDecode(unam);
			username = unam;
			crcut(username);
#ifdef _DEBUG
			fpo = fopen("uploadbuf", "a");
			if (fpo) fprintf(fpo, "username = %s\n", username);
			if (fpo) fclose(fpo);
#endif
		}
		else if (!strncmp(p, "password=", 9)) {
			strncpy(pswd, sp_pass(&p[9]) , 128);
			pswd[128] = '\0';
			com_urlDecode(pswd);
			password = pswd;
			crcut(password);
#ifdef _DEBUG
			fpo = fopen("uploadbuf", "a");
			if (fpo) fprintf(fpo, "password = [%s]\n", password);
			if (fpo) fclose(fpo);
#endif
		}
		else ;

		if (s == NULL) break;
		p = s + 1;
		n++;
	}

	return ((!pathname || !hostname || !username || !password)? -1: 0);
}

static int trsfile(char *sfile)
{
	char buf[BUFSIZ];
	FILE *fp;


	if ((fp = fopen(PASSWD_FILE, "w")) == NULL) {
		disp_html(ERR_SRC_HTML, "Disk Error");
	}
	fprintf(fp, "security_inf0=%s\n", username);
	fprintf(fp, "security_inf1=%s\n", password);
	fclose(fp);
		
#ifdef ENIREC
	sprintf(buf, "enigc -r -f %s -h %s -t \"%s\" >nul:", filename, hostname, pathname);
#else
	sprintf(buf, "enigc -f %s -h %s -t \"%s\" >nul:", filename, hostname, pathname);
#endif


#ifdef _DEBUG
	fp = fopen("uploadbuf", "a");
	if (fp) {
		fprintf(fp, "\n\n%s\n", buf);
		fclose(fp);
	}
#endif

	system(buf);

#ifdef _DEBUG
	fp = fopen("uploadbuf", "a");
	if (fp) {
		fprintf(fp, "enigc executed\n");
		fclose(fp);
	}
#endif
	_sleep(5);

	if ((fp = fopen(ERR_HANDLE, "r")) != NULL) {
		buf[0] = '\0';
		fgets(buf, BUFSIZ, fp);
		crcut(buf);
		disp_html(ERR_SRC_HTML, buf);
		fclose(fp);
		return -1;
	}
#ifdef _DEBUG
	fp = fopen("uploadbuf", "a");
	if (fp) {
		fprintf(fp, "Error Handle\n");
		fclose(fp);
	}
#endif


#ifdef ENIREC
//	system("extruct >nul:");
	system("xdel.exe var >nul:");
	system("cut_ktn10000.exe >nul:");
	system("tar xzf KTN10000.dat.tar.gz >nul:");
	system("xcopy /Y var\\spool\\nkycgi\\*.conf nkycgi\\  >nul:");
	unlink("KTN10000.dat.tar.gz");
#endif
	return 0;
}

int main(int argc, char *argv[])
{
	int s;

	printf("%s", html_str1);
	s = getform();
	if (s != 0)
		error(UPD2_ILLEGAL_STATUS);
	else s = trsfile(pathname);
	if (s == 0) disp_html(UPLOAD_DONE_HTML, "");

	exit(s);
}
