/******************************************************************
                                                                                
  Copyright (C) 2003-4 Nobby Noboru Hirano <nobby@aiware.jp>
  All Rights Reserved.
                                                                                
******************************************************************/
#define	ERR_SRC_HTML		"upload_error_src.html"
#define	UPLOAD_DONE_HTML	"upload_done.html"
#define	UPDATE_ING			"update_status_updating.html"
#define	DIST_HTML			"update_status.html"

#define	INBUFSIZ			(BUFSIZ*2)


enum {
	UPD2_ILLEGAL_FILE = 0,
	UPD2_SYSTEM_ERR,
	UPD2_UPLOAD_ERR,
	UPD2_MEMALOC_ERR,
	UPD2_ILLEGAL_HEADER,
	UPD2_ILLEGAL_STATUS,
	ERR_NUM_MAX
};

#ifdef USE_ERRMSG
#if 0
char *error_msg[ERR_NUM_MAX];
#else
char *error_msg[] = {
	"不正なファイルです。",
	"システム異常",
	"アップロードファイルが開けません",
	"メモリエラー",
	"ヘッダ異常",
	"アップデート異常終了",
	NULL
};
#endif
#endif

#define	ERROR_DEF_FILE		"ErroeDef.txt"

#define	scopy(x)			strcpy(xmalloc(strlen(x)+1),(x))

extern void crcut(char *s);
extern char *sp_pass(char *p);
extern void show_version(char *);
extern char *calc_md5_from_fp(FILE *, size_t);
extern char *calc_md5_from_file(char *);
extern char *MD5Digest(char *);
extern void com_urlDecode(char *);
