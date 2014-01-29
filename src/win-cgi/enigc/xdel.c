/***********************************************************
 Copyright 2004 by 
         Nobby N Hirano, All Rights Reserved

**
**
***********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/file.h>
#include <ctype.h>
#include <unistd.h>
// #include <pwd.h>
#include <io.h>

#define	Err(m)	{ perror(m); exit(1); }
#define scopy(x)	strcpy(xmalloc(strlen(x)+1),(x))
#define MAXDIRLEVEL	4096	/* BAD, it's possible to overflow this. */
#define MINIT		30	/* number of dir entries to initially allocate */
#define MINC		20	/* allocation increment */

#define	PATH_MAX	128

#ifndef TRUE
enum bool {FALSE=0, TRUE};
#endif

typedef unsigned char u_char;
typedef unsigned short u_short;

struct _info {
  char *name;
  char *lnk;
  u_char isdir  : 1;
  u_char issok  : 1;
  u_char isexe  : 1;
  u_char isfifo : 1;
  u_char orphan : 1;
  u_short mode, lnkmode, uid, gid;
  long long size;
  time_t atime, ctime, mtime;
  dev_t dev;
  ino_t inode;
};

char *ipattern = NULL, *pattern = NULL;

extern char *calc_md5_from_file(char *fnam);

/* Sorting functions */
int alnumsort(struct _info **a, struct _info **b)
{
	return strcmp((*a)->name,(*b)->name);
}

int (*cmpfunc)() = alnumsort;


/** Necessary only on systems without glibc **/
void *xmalloc (size_t size)
{
  register void *value = malloc (size);
  if (value == 0) {
    fprintf(stderr,"tree: virtual memory exhausted.\n");
    exit(1);
  }
  return value;
}

void *xrealloc (void *ptr, size_t size)
{
  register void *value = realloc (ptr,size);
  if (value == 0) {
    fprintf(stderr,"tree: virtual memory exhausted.\n");
    exit(1);
  }
  return value;
}

void free_dir(struct _info **d)
{
	int i;

	for(i=0;d[i];i++) {
		free(d[i]->name);
		if (d[i]->lnk) free(d[i]->lnk);
		free(d[i]);
	}
	free(d);
}

/*
 * Patmatch() code courtesy of Thomas Moore (dark@mama.indstate.edu)
 * returns:
 *    1 on a match
 *    0 on a mismatch
 *   -1 on a syntax error in the pattern
 */
int patmatch(char *buf, char *pat)
{
	int match = 1, m, n;

	while(*pat && match) {
		switch(*pat) {
		  case '[':
			pat++;
			if(*pat != '^') {
				n = 1;
				match = 0;
      } else {
				pat++;
				n = 0;
			}
			while(*pat != ']'){
				if(*pat == '\\') pat++;
				if(!*pat /* || *pat == '/' */ ) return -1;
				if(pat[1] == '-'){
					m = *pat;
					pat += 2;
					if(*pat == '\\' && *pat)
						pat++;
					if(*buf >= m && *buf <= *pat)
						match = n;
					if(!*pat)
						pat--;
				} else if(*buf == *pat) match = n;
				pat++;
			}
			buf++;
			break;
		  case '*':
			pat++;
			if(!*pat) return 1;
			while(*buf && !(match = patmatch(buf++,pat)));
			return match;
		  case '?':
			if(!*buf) return 0;
			buf++;
			break;
		  case '\\':
			if(*pat)
				pat++;
		  default:
			match = (*buf++ == *pat);
			break;
		}
		pat++;
		if(match<1) return match;
	}
	if(!*buf) return match;
	return 0;
}

struct _info **read_dir(char *dir, int *n)
{
	static char *path = NULL, *lbuf = NULL;
	static long pathsize = PATH_MAX+1, lbufsize = PATH_MAX+1;
	struct _info **dl;
	struct dirent *ent;
	struct stat lst,st;
	DIR *d;
	int ne, p = 0, rs;
	int lflag = TRUE;

	if (path == NULL) {
		path = xmalloc(pathsize);
		lbuf = xmalloc(lbufsize);
	}

	*n = 1;
	if ((d = opendir(dir)) == NULL) return NULL;

	dl = (struct _info **)xmalloc(sizeof(struct _info *) * (ne = MINIT));

	while((ent = (struct dirent *)readdir(d))) {
		if (!strcmp("..",ent->d_name) || !strcmp(".",ent->d_name)) continue;

		if (strlen(dir)+strlen(ent->d_name)+2 > pathsize) path = xrealloc(path,pathsize=(strlen(dir)+strlen(ent->d_name)+4096));
		sprintf(path,"%s/%s",dir,ent->d_name);
		if (stat(path,&lst) < 0) continue;
		if ((rs = stat(path,&st)) < 0) st.st_mode = 0;

		if ((lst.st_mode & S_IFMT) != S_IFDIR && lflag) {
			if (pattern && patmatch(ent->d_name,pattern) != 1) continue;
		}
		if (ipattern && patmatch(ent->d_name,ipattern) == 1) continue;

		if (pattern && !lflag) continue;


		if (p == (ne-1)) dl = (struct _info **)xrealloc(dl,sizeof(struct _info *) * (ne += MINC));
		dl[p] = (struct _info *)xmalloc(sizeof(struct _info));

		dl[p]->name = scopy(ent->d_name);
		dl[p]->mode = lst.st_mode;
		dl[p]->uid = lst.st_uid;
		dl[p]->gid = lst.st_gid;
		dl[p]->size = lst.st_size;
		dl[p]->dev = st.st_dev;
		dl[p]->inode = st.st_ino;
		dl[p]->lnk = NULL;
		dl[p]->orphan = FALSE;

		dl[p]->atime = lst.st_atime;
		dl[p]->ctime = lst.st_ctime;
		dl[p]->mtime = lst.st_mtime;

#if 0
		if ((lst.st_mode & S_IFMT) == S_IFLNK) {
			if (lst.st_size+1 > lbufsize) lbuf = xrealloc(lbuf,lbufsize=(lst.st_size+8192));
			if ((len=readlink(path,lbuf,lbufsize-1)) < 0) {
				dl[p]->lnk = scopy("[Error reading symbolic link information]");
				dl[p]->isdir = dl[p]->issok = dl[p]->isexe = FALSE;
				dl[p++]->lnkmode = st.st_mode;
				continue;
			} else {
				lbuf[len] = 0;
				dl[p]->lnk = scopy(lbuf);
				if (rs < 0) dl[p]->orphan = TRUE;
				dl[p]->lnkmode = st.st_mode;
			}
		}
#endif
    
		dl[p]->isdir = ((st.st_mode & S_IFMT) == S_IFDIR);
//		dl[p]->issok = ((st.st_mode & S_IFMT) == S_IFSOCK);
		dl[p]->isfifo = ((st.st_mode & S_IFMT) == S_IFIFO);
		dl[p++]->isexe = ((st.st_mode & S_IEXEC) | (st.st_mode & (S_IEXEC>>3)) | (st.st_mode & (S_IEXEC>>6))) ? 1 : 0;
	}
	closedir(d);
	*n = p;
	dl[p] = NULL;
	return dl;
}

void listdir(char *d, FILE *fp)
{
	char path[512], rpath[512];
#if 0
	long pathsize = 0;
#endif
	struct _info **dir, **sav;
	int n, i;
	char *s;
	static char *igdir[] = {
		"/nfs", "/dev", "/proc", "/opt", "/tmp", "/etc",
		 "nfs",  "dev",  "proc",  "opt",  "tmp",  "etc",
		NULL
	};

	sav = dir = read_dir(d, &n);
	if (!dir && n) Err("Dir read");
	if (!n) {
		free_dir(sav);
		return;
	}

	qsort(dir, n, sizeof(struct _info *), cmpfunc);

	for(i = 0;i < n;++i) {
#if 0
		if (sizeof(char) * (strlen(d)+strlen(dir[i]->name) + 2) > pathsize)
			path = xrealloc(path,pathsize=(sizeof(char) * (strlen(d)+strlen(dir[i]->name) + 1024)));
#endif
		if (!strncmp(d, "/", 1)) s = d + 1;
		else if (!strncmp(d, "./", 2)) s = d + 2;
		else s = d;
		sprintf(path,"%s/%s", s, dir[i]->name);

		sprintf(rpath,"%s/%s", d, dir[i]->name);

		if (dir[i]->isdir) {
			int j;
			int flg = 1;

			for(j = 0;igdir[j] != NULL;++j) {
				if (!strcmp(path, igdir[j])) {
					flg = 0;
					break;
				}
			}
			if (flg) listdir(rpath, fp);
		}
		else {
			if (dir[i]->lnk == NULL) {
//				printf("delete %s\n", rpath);
				unlink(rpath);
			}
		}
	}
//	free(path);
	free_dir(sav);

//	printf("delete dir %s\n", d);
	rmdir(d);
}

int main(int argc, char *argv[])
{
	int c;
	char *fnam = NULL;
	FILE *fp = stdout;
	char *_root = NULL;

	while ((c = getopt (argc, argv, "f:r:")) != EOF)
	switch (c) {
	  case 'f':
		fnam = optarg;
		break;
	  case 'r':
		_root = optarg;
		break;
	  default:
		break;
	}

	if (fnam != NULL) {
		if ((fp = fopen(fnam, "w")) == NULL)
			Err("Unable to open for write");
	}
	if (_root != NULL) {
		if (chdir(_root) < 0) Err("chdir");
	}

	if (optind < argc) {
		char *dnam = NULL;
		struct stat fs;

		while (optind < argc) {
			dnam = argv[optind++];

			if (stat(dnam, &fs) < 0) Err("stat");
			if (fs.st_mode & S_IFDIR) listdir(dnam, fp);
			else {
//				printf("delete %s\n", dnam);
				unlink(dnam);
			}
		}
	}
	else listdir("./", fp);
	if (fnam != NULL) fclose(fp);
	exit(0);
}
