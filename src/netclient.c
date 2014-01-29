/******************************************************************

  Copyright (C) 2003-2005
  Nobby Noboru Hirano <nobby@ntools.net>
  All Rights Reserved.

******************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <winsock.h>
#include <io.h>
#include <sys/stat.h>
#endif

#include "nsock.h"

#include "enge.h"

#ifdef __WIN32__
#define	PASSWD_FILE	"passwd"
#else
#define	PASSWD_FILE	"/var/spool/nkycgi/security_inf.conf"
#endif

#define	Err(m)	{ perror(m); exit(1); }

#define	INBUFSIZ	(BUFSIZ*2)
#define	PACK_SIZ	1024

#define	PORT_NO		Enigma_PORT

#define	ERR_HANDLE	"errfile"

static char *username = NULL;
static char *password = NULL;
static char *pathname = NULL;
static char *hostname = NULL;
static char *filename = "KTN10000.bin";

extern int randgen(unsigned long);
extern unsigned long hash(char *);



#ifndef __WIN32__
static int sig_rec = 0;

void exit_prog(int s)
{
	signal(SIGUSR1, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	sig_rec = 1;
}
#endif

void error_handle(char *msg)
{
	FILE *fp;

	if ((fp = fopen(ERR_HANDLE, "a")) == NULL) return;
	fprintf(fp, "%s\n", msg);
	fclose(fp);
}

static int get_id_psw(char **id, char **p)
{
	FILE *fp;
	char buf[128];
	static char unam[254], psw[254];

	if ((fp = fopen(PASSWD_FILE, "r")) == NULL) Err("passwd file");
	while(fgets(buf, 128, fp) != NULL) {
		crcut(buf);
		/*                 12345678901234 */
		if (!strncmp(buf, "security_inf0=", 14)) {
			strcpy(unam, &buf[14]);
			unam[253] = '\0';
			*id = unam;
		}
		else if (!strncmp(buf, "security_inf1=", 14)) {
			strcpy(psw, &buf[14]);
			psw[253] = '\0';
			*p = psw;
		}
		else ;
	}
	fclose(fp);
#ifdef _DEBUG
	fp = fopen("tekito2", "w");
	fprintf(fp, "id  = [%s]\n", *id);
	fprintf(fp, "psw = [%s]\n", *p);
	fclose(fp);
#endif
	return (*p != NULL && *id != NULL)? 0: -1;
}

static unsigned int atou(char *p, int cnt)
{
	unsigned int r = 0;

	while(*p != '\0' && cnt) {
		r *= 10;
		r += *p - '0';
		++p;
	}
	return (r);
}

int net_login(SOCKET sock, char *id, char *psw, int cnt)
{
	char *md5;
	unsigned int t;
	char buf[256];
#ifndef __WIN32__
	fd_set fds ;
#endif
#ifdef _DEBUG
	FILE *fp;
#endif


#ifndef __WIN32__
	SockStatus(sock, 10000, &fds);	/* 10 sec */
	if (!FD_ISSET(sock, &fds)) return -1;
#endif
//	t = (cnt != 0)? 132: 128;
//	if ((strlen(id) + strlen(psw)) > t) id = psw = "";

	buf[0] = '\0';
	SockGets(sock, buf, 256);
	t = atou(buf, 255);
	sprintf(buf, "%u%s%s", t, id, psw);
	md5 = MD5Digest(buf);
	SockPrintf(sock, "%s\n", md5);

#ifdef _DEBUG
	if (cnt) fp = fopen("tekito1", "a");
	else fp = fopen("tekito1", "w");
	fprintf(fp, "id  = [%s]\n", id);
	fprintf(fp, "psw = [%s]\n", psw);
	fclose(fp);
#endif

#ifndef __WIN32__
	SockStatus(sock, 10000, &fds);	/* 10 sec */
	if (!FD_ISSET(sock, &fds)) return -1;
#endif

	buf[0] = '\0';
	SockGets(sock, buf, 256);
	if (!strncmp(buf, "200", 3)) {
		randgen(hash(md5));
		return 0;
	}
	else if (!strncmp(buf, "402", 3)) {
		error_handle("他のユーティリティが使用中です。しばらく待って再度アクセスしてください。");
		return -1;
	}
	else if (cnt) error_handle("ログインに失敗しました。");
	else ;
	return -2;
}

static void *getform(void)
{
	char *buffer, *p;
	char *prm[10];
	size_t n, siz;

	siz = (size_t)atoi(getenv("CONTENT_LENGTH"));
	if((buffer = malloc(siz)) == NULL) {
		Err("Memory allocation error");
	}
	if((p = getenv("REQUEST_METHOD")) != NULL && !strcmp(p, "POST")) {
		n = fread( buffer, 1, (INBUFSIZ-1), stdin );
		buffer[ n ] = '\0' ;
	} else if((p = getenv("QUERY_STRING")) != NULL) {
		strcpy(buffer, p);
	} else ;

	p = prm[0] = buffer;

	n = 1;
	while((p = strchr(p, '&')) != NULL) {
		prm[n] = p + 1;
		*p = '\0';
		if(!strncmp(prm[n-1], "username=", 9)) username = sp_pass(&prm[n-1][9]);
		else if(!strncmp(prm[n-1], "password=", 9)) password = sp_pass(&prm[n-1][9]);
		else if(!strncmp(prm[n-1], "pathname=", 9)) pathname = sp_pass(&prm[n-1][9]);
		else if(!strncmp(prm[n-1], "hostname=", 9)) hostname = sp_pass(&prm[n-1][9]);

		if (hostname && username && password && pathname) break;
		p++;
		n++;
	}

	return ((void *)buffer);
}

static int recv_file(SOCKET sock, FILE *fp, char *fnam)
{
	unsigned char buf[PACK_SIZ];
	size_t cnt, bc;
#ifndef __WIN32__
	fd_set fds ;

	if (SockStatus(sock, 8000, &fds) < 0) {
		return -1;
	}
	if (!FD_ISSET(sock, &fds)) {
		return -1;
	}
#endif

	buf[0] = '\0';
	SockGets(sock, buf, PACK_SIZ - 1);

	if ((cnt = (size_t)atol(buf)) <= 0) return -1;

	while(cnt) {
#ifndef __WIN32__
		if (SockStatus(sock, 8000, &fds) < 0) return -1;
		if (FD_ISSET(sock, &fds)) {
#endif
			bc = (cnt > PACK_SIZ)? PACK_SIZ: cnt;
			if ((bc = EncRead(sock, buf, bc)) <= 0) return -1;
			fwrite(buf, sizeof(unsigned char), bc, fp);
			cnt -= bc;
#ifndef __WIN32__
		}
		else {			/* timeout */
			EncWrite(sock, "timeout\n", 8);
			return -2;
		}
#endif
	}
	SockWrite(sock, "end\n", 4);

	return 0;
}

static int send_file(SOCKET sock, FILE *fp, size_t cnt)
{
	unsigned char buf[PACK_SIZ];
	size_t bc;
#ifndef __WIN32__
	fd_set fds ;

	if (SockStatus(sock, 8000, &fds) == -1) return -1;
	if (!FD_ISSET(sock, &fds)) return -1;
#endif

	buf[0] = '\0';
	SockGets(sock, buf, PACK_SIZ-1);
	puts(buf);

	while(cnt) {
		bc = fread(buf, sizeof(unsigned char), PACK_SIZ, fp);
		if (ferror(fp) != 0) break;
		if (EncWrite(sock, buf, bc) <= 0) return -1;
		cnt -= bc;
	}
	SockWrite(sock, "end\n", 4);

	return 0;
}

#ifdef __WIN32__
void clean_sock(void)
{
	WSACleanup();
}
#endif

int main(int argc, char *argv[])
{
	int i, c = 0;
	int sock = -1;
	int mode_cgi = 0;
	int mode_rec = 0;
	int enc_flg = 0;
	int pnum = PORT_NO;
	FILE *fp = NULL;
	char *p;
	void *vp = NULL;
	char *ap = NULL;
	char psw[256], unam[256];

	unlink(ERR_HANDLE);
	p = strrchr(argv[0], '.');
	if (p != NULL) {
		ap = strdup(p);

		if (ap == NULL) Err("Memory allocation");
		strlwr(ap);
		if (!strcmp(ap, ".cgi")) mode_cgi = 1;
		free(ap);
	}

	if (mode_cgi) {
		p = strrchr(argv[0], '/');
		if (p == NULL) {
			if ((p = strrchr(argv[0], '\\')) == NULL) p = argv[0];
		}
		ap = strdup(p);
		if (ap == NULL) Err("Memory allocation");
		strlwr(ap);

		if (!strncmp(ap, "encrec", 6) ) mode_rec = 1;
		vp = getform();
	}
	else {
		for(i = 1;i < argc;++i) {
			p = argv[i];
			if(*p == '-') {
				++p;
				switch(*p) {
				  case 'e':
					enc_flg = 1;
					break;
				  case 'r':
					mode_rec = 1;
					break;
				  case 'p':
					pnum = atoi(argv[++i]);
					break;
				  case 'h':
					hostname = argv[++i];
					break;
				  case 'f':
					filename = argv[++i];
					break;
				  case 't':
					pathname = argv[++i];
					break;
				  case 'U':
					username = argv[++i];
					break;
				  case 'P':
					password = argv[++i];
					break;
				}
			}
			else hostname = argv[i];
		}
	}

#ifndef __WIN32__
	if (signal(SIGUSR1, exit_prog) == SIG_ERR) Err("Unable to set signal");
	if (signal(SIGHUP, exit_prog) == SIG_ERR) Err("Unable to set signal");
	if (signal(SIGINT, exit_prog) == SIG_ERR) Err("Unable to set signal");
	if (signal(SIGPIPE, exit_prog) == SIG_ERR) Err("Unable to set signal");
	if (signal(SIGTERM, exit_prog) == SIG_ERR) Err("Unable to set signal");
#endif

	if (mode_cgi && !pathname) {
		if (vp != NULL) free(vp);
		exit(1);
	}

	if (!username || !password) {
		if (get_id_psw(&username, &password) < 0) return -1;
	}

	if (!hostname || !username || !password || !pathname) {
		error_handle("システム異常");
		if (vp != NULL) free(vp);
		exit (1);
	}

	i = 0;
	while (1) {
		if ((sock = Socket(hostname, pnum)) <= 0) {
			error_handle("接続できません");
			if (vp != NULL) free(vp);
			exit(1);
		}


		if (i != 0) {
			if (strlen(password) > 250 || strlen(username) > 250) {
				closesocket(sock);
				if (vp != NULL) free(vp);
				exit(1);
			}
			sprintf(unam, "\"%s\"", username);
			sprintf(psw, "\"%s\"", password);
			psw[113] = '\0'; unam[113] = '\0';
#ifdef _DEBUG
			fp = fopen("tekito3", "w");
			fprintf(fp, "id  = [%s]\n", unam);
			fprintf(fp, "psw = [%s]\n", psw);
			fclose(fp);
#endif
			password = psw; username = unam;
		}
#ifdef __WIN32__
		else atexit(clean_sock);
#endif
		c = net_login(sock, username, password, i);
		if (c < -1) {
//			fprintf(stderr,"Login Fail\n");
			closesocket(sock);
			if (i++ < 1) continue;
			if (vp != NULL) free(vp);
			exit(1);
		}
		else if (c < 0) {
//			fprintf(stderr,"Login Busy.\n");
			closesocket(sock);
			if (vp != NULL) free(vp);
			exit(1);
		}
		else break;
	}

	if (enc_flg) SockPrintf(sock, "encr\n");
	else encryption_flag = 1;
//	usleep(10000);	/* 10ms */
	if (mode_rec) {
		if ((fp = fopen(pathname, "wb")) != NULL) {
			SockPrintf(sock, "recv /tmp/%s\n", filename);
			c = recv_file(sock, fp, filename);
			if (mode_cgi) system("extruct");
		}
		else {
			error_handle("システム異常");
			if (vp != NULL) free(vp);
			exit(1);
		}
	}
	else {
#ifdef __WIN32__
		struct _stat sb;
#else
		struct stat sb;
#endif

		if ((fp = fopen(pathname, "rb")) != NULL) {
#ifdef __WIN32__
			_fstat(fileno(fp), &sb);
#else
			fstat(fileno(fp), &sb);
#endif
			SockPrintf(sock, "send /tmp/%s\n", filename);
			SockPrintf(sock, "%d\n", sb.st_size);
			clearerr(fp);
			c = send_file(sock, fp, sb.st_size);
		}
	}

	if (fp != NULL) fclose(fp);
	closesocket(sock);
	if (vp != NULL) free(vp);
	exit((c == 0)? 0: 1);
}
