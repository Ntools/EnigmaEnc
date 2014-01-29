#include <stdio.h>
#include <stdlib.h>

int cat (char *nam)
{
	FILE *fp;
	int c;

	if ((fp = fopen(nam, "rb")) == NULL) return -1;

	while((c = fgetc(fp)) != EOF) fputc(c, stdout);
	fclose(fp);
	fflush(stdout);
	return 0;
}

int main(int argc, char *argv[])
{
	int i;

	for(i = 1;i < argc;++i) {
		cat(argv[i]);
	}
	exit(0);
}
