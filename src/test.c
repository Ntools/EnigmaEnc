#include <stdio.h>
#include <string.h>

extern int randgen(unsigned long);
extern unsigned long hash(char *);
extern int bufencode(unsigned char *, const unsigned char *, size_t);
extern int bufdecode(unsigned char *, const unsigned char *, size_t);
extern unsigned char eng_gear_sel;

int main(int argc, char *argv[])
{
	unsigned long sd;
	unsigned char buf[BUFSIZ];
	unsigned char dbuf[BUFSIZ];
	int cnt;
	unsigned char *src = "ABC defghijk LMN OPQRSTUVWXYZ";

	if (argc > 1) sd = hash(argv[1]);
	else sd = 19650218;

	printf("Seed fuctor is %lu\n", sd);
	randgen(sd);

	cnt = bufencode(buf, src, strlen(src));
	eng_gear_sel = 0;
	cnt = bufdecode(dbuf, buf, cnt);
	dbuf[cnt] = 0;
	if (!strcmp(dbuf, src)) {
		puts("Test successed.");
		exit(0);
	}
	else {
		puts("NG !!");
		printf ("src = [%s] result = [%s]", src, dbuf);
		exit(1);
	}
}
