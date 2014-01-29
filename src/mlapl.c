/***************************************************************************
 * Copyright 1996-98 Nobby Noboru Hirano
 * All rights reserved
 *
 * exclude executable files, Under working following OS
 *                          Linux   ELF 2.0.0 or later
 *
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Nobby Noboru Hirano <nobby@nmail.hiug.jp>
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nmail.h>

#define	INBUFSIZ	65536

#define MEMBER_DIR	"/var/spool/ml"

static char *mailadrs = NULL;
static char *mailcfms = NULL;
static char *fromname = NULL;
static char *mlname = NULL;
static char *mlreturn = NULL;
static char *msgto = NULL;

static char *sp_pass(char *p)
{
	while(*p == ' ' || *p == '\t') ++p;
	crcut(p);
	return(p);
}

static void error(char *msg)
{
	printf("Content-type: text/html\n\n");
	printf("<HTML><HEAD><TITLE>HIUG Service</TITLE>\n");
	printf("<META HTTP-EQUIV=Content-Type CONTENT=\"text/html; charset=euc-jp\">\n");
	printf("</HEAD>\n");
	printf("<BODY BGCOLOR=\"#707088\" TEXT=\"#ffbb00\" LINK=\"#ffff00\" ALINK=\"#ff0000\" VLINK=\"#ffcccc\">\n");
	printf("<PRE><FONT SIZE=5 COLOR=\"RED\">%s\n",msg);
	printf("</PRE></FONT>\n");
	printf("<P><HR></P>\n");
	if(mlreturn != NULL) {
		printf("<A HREF=\"%s\">\n", mlreturn);
		printf("<IMG SRC=\"http://www.hiug.ne.jp/icons/return.gif\" ALT=\"Back\" BORDER=0>\n");
		printf(" 戻る</A>\n");
	}
	printf("</BODY></HTML>\n");
	exit(0);
}

static int hex16(int c)
{
	return((c > '9')? (toupper(c) - 'A' + 10): (c - '0'));
}

static int pkconv(char *pd)
{
	char *buf, *p;
	int i;

	if((buf = malloc(INBUFSIZ)) == NULL) {
		error("Memory allocation error");
	}
	p = pd;
	for(i = 0;*p != '\0';++i) {
		if(p[0] == '%' && isxdigit(p[1]) && isxdigit(p[2])) {
			buf[i] = (16 * hex16(p[1])) + hex16(p[2]);
			p = &p[3];
		}
		else if(p[0] == '+') {
			buf[i] = ' ';
			++p;
		}
		else buf[i] = *p++;
	}
	buf[i] = '\0';
	cnvbuff(buf);
	strcpy(pd, buf);
}

static int chk7bit(char *p)
{
	int i;

	for(i = 0;p[i] != '\0';++i) {
		if(p[i] & 0x80)
			error(
		"入力エラー: メールアドレスに異常な文字(漢字?)が使われています。\n"
		"           メールアドレスには全角文字は使用できません。");
	}
	return 0;
}

static void mail_send(char *to, char *val[], int ac)
{
	FILE *fp;
	char cmd[BUFSIZ];
	char *prm[10];
	int i;

	if((fp = fopen("/tmp/kodai.tmp", "w")) == NULL) return;
	for(i = 0;i < ac;++i) fprintf(fp, "%s\n", val[i]);
	fclose(fp);
#if 0
	sprintf(cmd, "-S smtp.hiug.ne.jp -F Mailinglist -s \"ML Attach Result\" -f /tmp/kodai.tmp %s", to);
	mailsend(cmd);
/* #else */
	prm[0] = "-S"; prm[1] = "localhost";
	prm[2] = "-F"; prm[3] = "Mailinglist";
	prm[4] = "-s"; prm[5] = "ML Attach Result";
	prm[6] = "-f"; prm[7] = "/tmp/kodai.tmp";
	prm[8] = to;
	smail(9, prm);
#endif
}

static void getform(void)
{
	char *buffer, *p;
	char *prm[10];
	size_t n, i;
	FILE *fp;
	unsigned char *buf;
	int smtpmain(int, char **);

	if((buffer = malloc(INBUFSIZ)) == NULL) {
		error("Memory allocation error");
	}
	if((p = getenv("REQUEST_METHOD")) != NULL && !strcmp(p, "POST")) {
		n = fread( buffer, 1, (INBUFSIZ-1), stdin );
		buffer[ n ] = '\0' ;
	} else if((p = getenv("QUERY_STRING")) != NULL) {
		strcpy(buffer, p);
	} else ;

	outputmode = MODE_EUC;
	pkconv(buffer);

	p = prm[0] = buffer;

	n = 1;
	while((p = strchr(p, '&')) != NULL) {
		prm[n++] = p + 1;
		*p = '\0';
		p++;
	}
	for(i = 0;i < n;++i) {
		if(!strncmp(prm[i], "mailadrs=", 9)) mailadrs = sp_pass(&prm[i][9]);
		else if(!strncmp(prm[i], "mailcfms=", 9)) mailcfms = sp_pass(&prm[i][9]);
		else if(!strncmp(prm[i], "name=", 5)) fromname = sp_pass(&prm[i][5]);
		else if(!strncmp(prm[i], "mlname=", 7)) mlname = sp_pass(&prm[i][7]);
		else if(!strncmp(prm[i], "return=", 7)) mlreturn = sp_pass(&prm[i][7]);
		else if(!strncmp(prm[i], "mail2=", 6)) msgto = sp_pass(&prm[i][6]);
	}

	if(mlname == NULL || *mlname == '\0')
		error("error: Form HTML file, No ML-name.");
	else if(mailadrs == NULL || *mailadrs == '\0')
		error("入力エラー: メールアドレスが記入洩れです。");
	else if(mailcfms == NULL || *mailcfms == '\0')
		error("入力エラー: 確認用メールアドレスが記入洩れです。");
	else if(strcmp(mailadrs, mailcfms) != 0)
		error("入力エラー: メールアドレスが不一致で確認できません。");
	chk7bit(mailadrs);
	if(msgto != NULL) mail_send(msgto, prm, n);
}

static void makehtml(void)
{
	printf("Content-type: text/html\n\n");
	printf("<HTML><HEAD>\n");
	printf("<TITLE>Nmail ML</TITLE>\n");
	printf("<META HTTP-EQUIV=Content-Type CONTENT=\"text/html; charset=euc-jp\">\n");
	printf("</HEAD>\n");
	printf("<BODY BGCOLOR=\"#707088\" TEXT=\"#ffbb00\" LINK=\"#ffff00\" ALINK=\"#ff0000\" VLINK=\"#ffcccc\">\n");
	printf("<P><FONT SIZE=5>\n");
	printf("処理は正常に終了しました。\n</FONT>");
	printf("<P><HR></P>\n");
	if(mlreturn != NULL) {
		printf("<A HREF=\"%s\">\n", mlreturn);
		printf("<IMG SRC=\"http://www.hiug.ne.jp/icons/return.gif\" ALT=\"Back\" BORDER=0>\n");
		printf(" 戻る</A>\n");
	}
	printf("</BODY>\n");
	printf("</HTML>\n");
}

static void update(void)
{
	FILE *fp[3];
	char fnam[3][256];
	char buf[BUFSIZ];
	int i;

	sprintf(fnam[0], MEMBER_DIR "/%s/actives", mlname);
	sprintf(fnam[1], MEMBER_DIR "/%s/members", mlname);
	sprintf(fnam[2], MEMBER_DIR "/%s/addcgi.log", mlname);

	/* check double regist */
	if((fp[0] = fopen(fnam[1], "r")) == NULL)
		error("システム異常: 申し訳ありませんが何らかの原因で処理できません.");
	while(fgets(buf, BUFSIZ, fp[0]) != NULL) {
		crcut(buf);
		if(!strcmp(buf, mailadrs)) error("エラー: 二重登録です。");
	}
	fclose(fp[0]);
	for(i = 0;i < 3;++i) {
		if((fp[i] = fopen(fnam[i], "a")) == NULL)
			error("システム異常: 申し訳ありませんが何らかの原因で処理できません.");
	}
	for(i = 0;i < 2;++i) {
		fprintf(fp[i], "%s\n", mailadrs);
		fclose(fp[i]);
	}
	fprintf(fp[i], "%s <%s> ", fromname, mailadrs);
	fprintf(fp[i], "Blowser = %s ", getenv("HTTP_USER_AGENT"));
	fprintf(fp[i], "Remote_Host = %s [%s]\n", getenv("REMOTE_HOST"), getenv("REMOTE_ADDR"));
	fclose(fp[i]);
}

main(void)
{
	char *p, cmd[BUFSIZ];

	getform();
	update();
	makehtml();
	fflush(stdout);
	if(msgto != NULL) {
		sprintf(cmd, "/usr/bin/sendmime -S smtp.hiug.ne.jp -F Mailinglist -s \"ML Attach Result\" -f /tmp/kodai.tmp %s", msgto );
		system(cmd);
		puts(cmd);
/*		unlink("/tmp/kodai.tmp"); */
	}
	exit(0);
}
