/******************************************************************

  Copyright (C) 2003-2005
  Nobby Noboru Hirano <nobby@ntools.net>
  All Rights Reserved.

******************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>

#include <nsock.h>

#include "enge.h"

#ifdef __PPC__
#define	PASSWD_FILE	"/var/spool/nkycgi/security_inf.conf"
#else
#define	PASSWD_FILE	"/var/enigma/etc/passwd"
#endif

#define	LOCK_FILE	"/var/run/exclusive"
#define	WEB_LOGIN	"/var/spool/nkycgi/sid.conf"

#define	Err(m)	{ perror(m); exit(1); }

#define	PORT_NO		Enigma_PORT
#define	PACK_SIZ	1024

static int sig_rec = 0;

extern int randgen(unsigned long);
extern unsigned long hash(char *);


void exit_prog(int s)
{
	signal(SIGUSR1, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	sig_rec = 1;
}

#if 0
static char *dccut_dup(char *buf)
{
	char bf[512];
	int i, j;

	for(j = i = 0;i < 512;++i) {
		if (buf[i] == '\0') break;
		if (buf[i] == '\"') continue;
		bf[j++] = buf[i];
	}
	bf[j] = '\0';
	return strdup(bf);
}
#endif

static int get_id_psw(char **p, char **id)
{
	FILE *fp;
	char buf[128];

	if ((fp = fopen(PASSWD_FILE, "r")) == NULL) Err("passwd file");
	while(fgets(buf, 128, fp) != NULL) {
		crcut(buf);
		/*                 12345678901234 */
		if (!strncmp(buf, "security_inf0=", 14)) *id = strdup(&buf[14]);
		else if (!strncmp(buf, "security_inf1=", 14)) *p = strdup(&buf[14]);
		else ;
	}
	fclose(fp);
#ifdef _DEBUG
	printf("id  = [%s]\n", *id);
	printf("psw = [%s]\n", *p);
#endif
	return (*p != NULL && *id != NULL)? 0: -1;
}

int net_login(SOCKET sock)
{
	static char *psw = NULL, *id = NULL;
	char *md5;
	time_t t;
	char buf[256];
	fd_set fds ;

	if (psw == NULL) {
		if (get_id_psw(&psw, &id) < 0) return FALSE;
	}
	t = time(NULL);
	sprintf(buf, "%u%s%s", (unsigned int)t, id, psw);
	md5 = MD5Digest(buf);
	SockPrintf(sock, "%d\n", t);
	/* wait MD5 password until time out */
	SockStatus(sock, 10000, &fds);	/* 10 sec */
	if (FD_ISSET(sock, &fds)) {
		SockGets(sock, buf, 255);
		crcut(buf);
#ifdef _DEBUG
		printf("rx=[%s] tx=[%s]\n", buf, md5);
#endif
		if (!strcmp(buf, md5)) {
			randgen(hash(md5));
			return TRUE;	/* login OK */
		}
	}
	SockPrintf(sock, "401 Unauth\n");
	return FALSE;
}

static int tar_gz(char *arc)
{
	FILE *pi, *po;
	int c, cnt = 0;
	char buf[512];

	chdir("/");
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

static int targz(char *arc)
{
	char *p;
	char buf[34];
	int i;
	FILE *fp, *fo;

	if ((fp = fopen(arc, "rb")) == NULL) {
		fprintf(stderr, "Unable to open %s\n", arc);
		return -1;
	}
	for(i = 0;i < 33;++i) buf[i] = fgetc(fp);
	buf[32] = '\0';
	p = calc_md5_from_fp(fp, -1);
	fclose(fp);
	if (p == NULL || strncmp(buf, p, 32)) {
		fprintf(stderr, "%s\n%s\n", p, buf);
		fprintf(stderr, "MD5 Error\n");
		return -2;
	}

	if ((fp = fopen(arc, "rb")) == NULL) {
		return -1;
	}
	fseek(fp, 33, SEEK_SET);

	if ((fo = fopen("/tmp/tz", "wb")) == NULL) {
		return -1;
	}
	while((i = getc(fp)) != EOF) {
		fputc (i, fo);
	}
	fclose(fp); fclose(fo);
#ifdef _DEBUG
	system("ls -al /tmp");
#endif

	i = tar_gz("/tmp/tz");
	if (i < 0) return -1;
	system("/var/spool/nkycgi/sh/lateReboot.sh >/dev/null 2>&1 &");
	exit(0);
}

static int send_file(SOCKET sock, char *fnam)
{
	FILE *fp;
	unsigned char buf[PACK_SIZ];
	size_t bc;
	fd_set fds ;

#ifdef __WIN32__
	struct _stat sb;
#else
	struct stat sb;
#endif

	if ((fp = fopen(fnam, "rb")) == NULL) return -1;
#ifdef __WIN32__
	_fstat(fileno(fp), &sb);
#else
	fstat(fileno(fp), &sb);
#endif

	SockPrintf(sock, "%d\n", sb.st_size);

	while(sb.st_size) {
		bc = (sb.st_size > PACK_SIZ)? PACK_SIZ: sb.st_size;
		bc = fread(buf, sizeof(unsigned char), bc, fp);
		EncWrite(sock, buf, bc);
		sb.st_size -= bc;
	}
	fclose(fp);
	if (SockStatus(sock, 8000, &fds) < 0) return -1;
	if (!FD_ISSET(sock, &fds)) return -1;
	SockGets(sock, buf, PACK_SIZ - 1);
	return 0;
}

static int recv_file(SOCKET sock, char *fnam)
{
	FILE *fp;
	unsigned char buf[PACK_SIZ];
	size_t cnt, bc, ttl = 0;
	fd_set fds ;
	int st = 0;

	if (SockStatus(sock, 8000, &fds) < 0) return -1;
	if (!FD_ISSET(sock, &fds)) return -1;
	buf[0] = '\0';
	SockGets(sock, buf, PACK_SIZ - 1);

	if ((cnt = (size_t)atol(buf)) <= 0) return -1;

	if ((fp = fopen(fnam, "wb")) == NULL) return -1;

	SockPrintf(sock, "start\n");

	while(cnt) {
		if (SockStatus(sock, 8000, &fds) < 0) {
			st = -1;
			break;
		}
		if (FD_ISSET(sock, &fds)) {
			bc = (cnt > PACK_SIZ)? PACK_SIZ: cnt;
			if ((bc = EncRead(sock, buf, bc)) <= 0) return -1;
			fwrite(buf, sizeof(unsigned char), bc, fp);
			cnt -= bc;
			ttl += bc;
		}
		else {      /* timeout */
//			EncWrite(sock, "timeout\n", 8);
			fprintf(stderr," Recv timeout\n");
			st = -2;
			break;
		}
	}
	fclose(fp);
	if (!strcmp(fnam, "/tmp/update2.dat")) {
#ifdef _DEBUG
		puts("update2 execute");
#endif
		system("update2");
		system("Reboot");
	}
	else if (!strcmp(fnam, "/tmp/KTN10000.bin")) {
#ifdef _DEBUG
		puts("tar zxf /tmp/KTN10000.bin execute");
#endif
		targz(fnam);
		system("Reboot");
	}
#ifdef _DEBUG
	else printf("Unknown file %s", fnam);
#endif

	SockWrite(sock, "end\n", 4);

	return st;
}

int svr_(SOCKET sock[])
{
	int netsock = -1, i;
	fd_set fds ;
	char *ip = NULL, buf[256], *p;

	while(!sig_rec) {
		if (netsock < 0) {
			for (i = 0;i < 2;++i) {
				SockStatus(sock[i], 0, &fds);
				if (FD_ISSET(sock[i], &fds)) {
					if ((netsock = act_rec(sock[i], &ip)) < 0) continue;
					if (net_login(netsock) == FALSE) {
#ifdef _DEBUG
						printf("Login auth fail %s\n", ip) ;
#endif
						closesocket(netsock);
						netsock = -1;
					}
					else { /* login success */
						FILE *fp;
						struct stat sb;
						time_t t;

#ifdef _DEBUG
						printf("Login From %s\n", ip) ;
#endif
						encryption_flag = 1;
						if (stat(WEB_LOGIN, &sb) == 0) {
							t = time(NULL);
							t = (t > sb.st_mtime)? (t - sb.st_mtime): 700;
							if (t < 600) {	/* withen 10 min. */
#ifdef _DEBUG
								printf("Login refuse. Webmin is used\n") ;
#endif
								SockPrintf(netsock, "402 Process busy\n");
								closesocket(netsock);
								netsock = -1;
								usleep(50000);	/* wait for client close */
								continue;
							}
						}
						SockPrintf(netsock, "200 OK\n");
						if ((fp = fopen(LOCK_FILE, "wb")) != NULL) fclose(fp);
					}
				}
			}
		}
		else {
			/* RX from client ? */
			if (SockStatus(netsock, 10000, &fds) == -1) ;

			else if (FD_ISSET(netsock, &fds)) {
				if (SockGets(netsock, buf, 255) > 0) {
#ifdef _DEBUG
					puts(buf);
#endif
					if (!strncmp(buf, "recv ", 5)) {
						p = sp_pass(&buf[5]);
						send_file(netsock, p);
					}
					else if (!strncmp(buf, "send ", 5)) {
						p = sp_pass(&buf[5]);
						if (!strncmp(p, "/tmp/", 5))
							recv_file(netsock, p);
					}
					else if (!strncmp(buf, "encr", 4)) {
						printf("encryption off\n");
						encryption_flag = 0;
						continue;
					}
				}
			}
			closesocket(netsock);
			netsock = -1;
			unlink(LOCK_FILE);
			usleep(50000);	/* wait for client close */
		}
	}

	return 0;
}


void usage(int e, char *nam)
{
	show_version(nam);
	fprintf(stderr, "option:\n"
	"       P <portnum>   Portnumber\n"
	);
	exit(e);
}

int main(int argc, char *argv[])
{
	int c;
	int pnum = PORT_NO;
	int sock[] = {-1, -1};
	char *hnam = NULL;
#ifndef _DEBUG
	pid_t pid;
#endif

	while ((c = getopt (argc, argv, "p:P:h:")) != EOF)
	switch (c) {
	  case 'p':
	  case 'P':
		pnum = atoi(optarg);
		break;
	  case 'h':
		hnam = optarg;
		break;
	  default:
		usage(1, basename(argv[0]));
		break;
	}
#if 0
	if (optind > argc - 1) req = 3;
	else req = atoi(argv[optind]);

	if (req < 1 || req > 5) {
		fprintf(stderr, "Illegal Video Mode");
		exit(1);
	}
#endif

#ifndef _DEBUG
	if ((pid = fork()) == -1) perror("could not fork");
	if (pid != 0) exit(0);
#endif

    if (signal(SIGUSR1, exit_prog) == SIG_ERR) Err("Unable to set signal");
    if (signal(SIGHUP, exit_prog) == SIG_ERR) Err("Unable to set signal");
	if (signal(SIGTERM, exit_prog) == SIG_ERR) Err("Unable to set signal");

	if (optind >= argc) {
		fprintf(stderr, "Can not get hostname !\n\a");
		exit (1);
	}

	c = 0;
	while (optind < argc) {
		if ((sock[c] = Ssocket(argv[optind], pnum)) <= 0) {
			sleep(60);
			continue;
		}
		++c;  optind++;
		if (c >= 2) break;
	}

	c = svr_(sock);
	closesocket(sock[0]);
	closesocket(sock[1]);

	exit((c == 0)? 0: 1);
}
