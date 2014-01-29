/******************************************************************
                                                                                
  Copyright (C) 2003-4 Nobby Noboru Hirano <nobby@aiware.jp>
  All Rights Reserved.
                                                                                
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef WIN32
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mmap.h"
#endif

#define	INBUFSIZ	1024
#define	ERR_SRC_HTML			"/home/httpd/html/cgi-bin/upload_error_src.html"
#define	UPLOAD_DONE_HTML		"/home/httpd/html/cgi-bin/upload_done.html"
#define	UPDATE_ING			"/home/httpd/html/cgi-bin/update_status_updating.html"
#define	DIST_HTML			"/home/httpd/html/cgi-bin/update_status.html"
#define	ERROR_DEF_FILE		"ErroeDef.txt"

#define	scopy(x)			strcpy(xmalloc(strlen(x)+1),(x))

static char *html_str1 = "Content-type: text/html\n\n";

static char *html_str2 = "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0;URL=update_status.html\">\n";
static int update_f = 1;

enum {
	UPD2_ILLEGAL_FILE = 0,
	UPD2_SYSTEM_ERR,
	UPD2_UPLOAD_ERR,
	UPD2_MEMALOC_ERR,
	UPD2_ILLEGAL_HEADER,
	UPD2_ILLEGAL_STATUS,
	ERR_NUM_MAX
};

static char *error_msg[ERR_NUM_MAX];

extern char *sp_pass(char *);
extern char *crcut(char *);

void *xmalloc (size_t size)
{
	void *value = malloc (size);

	if (value == NULL) {
		fprintf(stderr,"tree: virtual memory exhausted.\n");
		exit(1);
	}
	return value;
}

static void disp_html(char *fnam, char *msg)
{
	FILE *fp;
	char buf[512], *p;

	if ((fp = fopen(fnam, "r")) == NULL) {
		fprintf(stderr, "What can I do ??\n");
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

static char *word(char *p, char **s)
{
	char buf[256];
	int i = 0;

	while(*p > ' ') buf[i++] = *p++;
	buf[i] = '\0';
	if (i > 0) *s = scopy(buf);
	else *s = NULL;
	return (p);
}

static char *digip(char *p, int *n)
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
		p = digip(p, &n);
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

static void error(int n)
{
	disp_html(ERR_SRC_HTML, error_msg[n]);
	exit(0);
}

#ifdef WIN32
#define	spawn_prog system
#else
static int spawn_prog(const char *pr)
{
	pid_t cpid;
	char *argv[32], *p, *d;
	char prm[BUFSIZ];
	int i;

	strcpy(prm, pr);
	d = " ,";

	p = strtok(prm, d);
	for(i = 0;i < 31;++i) {
		if(p == NULL) break;
		argv[i] = p;
		p = strtok(NULL, d);
	}
	argv[i] = NULL;

	if ((cpid = fork()) == 0) {
		execvp (argv[0], argv);
		return (-1);
	}
	wait(&i);
	return (i);
}
#endif

static void cphtml(char *src)
{
	char cpb[512];

	sprintf(cpb, "cp %s " DIST_HTML , src);
	spawn_prog(cpb);
}

static char *find_mime(char *p)
{
	if ((p = strstr(p, "Content-Disposition: form-data;")) != NULL) p += 31;
	return (p);
}

static char *find_crlf(char *p)
{
	if ((p = strstr(p, "\r\n\r\n")) != NULL) p += 4;
	return (p);
}

static void memtofile(int siz, char *st, char *ed, char *fnam)
{
	FILE *fp;
	size_t cnt;
	int c, ct;
	static char edl[] = "\r\n-------------------";

	cnt = (ed - st);

	if ((fp = fopen(fnam, "wb")) == NULL)
		error(UPD2_UPLOAD_ERR);
	fwrite(st, 1, cnt, fp);
	ct = 0;
	while(siz--) {
		if ((c = fgetc(stdin)) == EOF) break;
		if (edl[ct] == '\0') break;
		if (c == edl[ct]) ++ct;
		else {
			if (ct != 0) {
				int i;

				for(i = 0;i < ct;++i) fputc(edl[i], fp);
				ct = 0;
			}
			fputc(c, fp);
		}
	}
	fclose(fp);
}


#ifdef WIN32
static int tar_gz (char *arc)
{
	char buf[BUFSIZ];

	sprintf(buf, "tar %s", arc);
	return system(buf);
}
#else
static int tar_gz(char *arc)
{
	FILE *pi, *po;
	int c, cnt = 0;
	char buf[512];

	sprintf(buf, "gunzip -c %s 2>/dev/null", arc);	/* Error out to null */
	if ((pi = popen(buf , "r")) == NULL) {
		fprintf(stderr, "pipe Akimahen !!\n");
		return -2;
	}


	strcpy (buf, "tar xf - >/dev/null 2>&1");
	if ((po = popen(buf, "w")) == NULL) {
		fprintf(stderr, "pipe Akimahen !!\n");
		pclose(pi);
		return -3;
	}

	while((c = getc(pi)) != EOF) {
		fputc (c, po);
		++cnt;
	}
	pclose(po); pclose(pi);
	return ((cnt < 512)? -1: 0);
}
#endif

static int targz(char *arc)
{
	char *p;
	char buf[34];
	int i;
	FILE *fp, *fo;

	if ((fp = fopen(arc, "rb")) == NULL) {
		error(UPD2_ILLEGAL_FILE);
		return -1;
	}
	for(i = 0;i < 33;++i) buf[i] = fgetc(fp);
	buf[32] = '\0';
	p = calc_md5_from_fp(fp, -1);
	fclose(fp);
	if (p == NULL || strncmp(buf, p, 32)) {
		error(UPD2_ILLEGAL_FILE);
		return -2;
	}

	if ((fp = fopen(arc, "rb")) == NULL) {
		error(UPD2_ILLEGAL_FILE);
		return -1;
	}
	fseek(fp, 33, SEEK_SET);

	p = "/tmp/.tgz";
	if ((fo = fopen(p, "wb")) == NULL) {
		error(UPD2_ILLEGAL_FILE);
		return -1;
	}
	while((i = getc(fp)) != EOF) {
		fputc (i, fo);
	}
	fclose(fp); fclose(fo);

	return (tar_gz(p));
}

static int getform(char *fnam)
{
	char *buffer, *p, *pe;
	int siz, st = 0;
	size_t n;
#ifdef _DEBUG_
	FILE *fpo;
#endif

	if((p = getenv("REQUEST_METHOD")) == NULL || strcmp(p, "POST"))
		error(UPD2_SYSTEM_ERR);

	siz = atoi(getenv("CONTENT_LENGTH"));
	if (siz < INBUFSIZ) {
		if (siz < 512) siz = 512;
		if((buffer = malloc(siz)) == NULL) {
			error(UPD2_MEMALOC_ERR);
		}
		n = fread( buffer, 1, siz, stdin );
		siz = 0;
	}
	else {
		if((buffer = malloc(INBUFSIZ)) == NULL) {
			error(UPD2_MEMALOC_ERR);
		}
		n = fread( buffer, 1, INBUFSIZ, stdin );
		siz -= INBUFSIZ;
	}
#if 0
	fpo = fopen("/var/log/uploadbuf", "wb");
	if (fpo) {
		fwrite(buffer, 1, n, fpo);
		fclose(fpo);
	}
#endif

	pe = &buffer[n];

#ifdef _DEBUG_
	fpo = fopen("/var/log/upload.log", "w");
#endif
	if ((p = find_mime(buffer)) == NULL) {
#ifdef _DEBUG_
		if (fpo) fprintf(fpo, "Header Error1\n");
#endif
		free(buffer);
		error(UPD2_ILLEGAL_HEADER);
	}
	if ((p = find_crlf(p)) == NULL) {
#ifdef _DEBUG_
		if (fpo) fprintf(fpo, "Header Error2\n");
#endif
		free(buffer);
		error(UPD2_ILLEGAL_HEADER);
	}

#ifdef _DEBUG_
	if (fpo) fprintf(fpo, "write fname %s\n", fnam);
#endif
	memtofile(siz, p, pe, fnam);

	if (strstr(fnam, "KTN10000") == NULL) {
		cphtml(UPDATE_ING);
		printf("%s", html_str2); fflush(stdout);
//		st = spawn_prog("update2", 0);
		system("update2 >/dev/null 2>&1 &");
	}
	else {
		chdir("/");
		if (targz(fnam) != 0)
			error(UPD2_ILLEGAL_FILE);
	}
#ifdef _DEBUG_
	if (fpo) fclose(fpo);
#endif
	free(buffer);
	return st;
}

int main(int argc, char *argv[])
{
	int s;
#ifdef WIN32
	char *fnam = "KTN10000.dat";
#else
	int c;
	char *fnam = "/tmp/update2.dat";

	setenv("PATH", "/usr/sbin:/usr/bin:/sbin:/bin:/home/httpd/html/cgi-bin:/cgi-bin", 1);
	if(!strncmp(argv[0], "setupload", 9)) {
		fnam = "/tmp/KTN10000.dat";
		update_f = 0;
	}

	while ((c = getopt (argc, argv, "f:")) != EOF)
	switch (c) {
	  case 'f':
		fnam = optarg;
		break;
	  default:
		fprintf(stderr, "Unknown option -%c \n", c);
		exit(1);
		break;
	}
#endif
	error_msg_init();

	printf("%s", html_str1); fflush(stdout);
	s = getform(fnam);
	if (s)
		error(UPD2_ILLEGAL_STATUS);
	if (!update_f) disp_html(UPLOAD_DONE_HTML, "");
	free_err_msg();
	exit(s);
}
