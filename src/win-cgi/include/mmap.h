/***********************************************************
 Copyright 2004-5 by 
         Nobby N Hirano All Rights Reserved

**
** autoupdate
**
**
**
***********************************************************/
#define	SVR_FQDN		"update.ntools.net"
#define	SVR_LOOKUP		"nslookup update.ntools.net"
#define	FILE_HTML_SCR	"diskimage_html_sh.tar.gz"

#define	VER_FILE		"version"
#define	MD5_FILE		"md5calc"
#define	IMG_FILE		"DImage"

#define FLASH2_PHYS_ADDR 0xffc00000
#define FLASH_SIZE 0x400000
                                                                                
#define FLASH_PARTITION0_ADDR 0x00000000
#define FLASH_PARTITION0_SIZE 0x00002000
                                                                                
#define FLASH_PARTITION1_ADDR 0x00002000
#define FLASH_PARTITION1_SIZE 0x0000E000
                                                                                
#define FLASH_PARTITION2_ADDR 0x00010000
#define FLASH_PARTITION2_SIZE 0x002F0000
                                                                                
#define FLASH_PARTITION3_ADDR 0x00300000
#define FLASH_PARTITION3_SIZE 0x00040000
                                                                                
#define FLASH_PARTITION4_ADDR 0x00340000
#define FLASH_PARTITION4_SIZE 0x000C0000

#define FLASH1_PHYS_ADDR	0xff400000

#define	FLASH_PARTITION5_ADDR	0x00000000
#define	FLASH_PARTITION5_SIZE	0x00280000

#define	FLASH_PARTITION6_ADDR	0x00280000
#define	FLASH_PARTITION6_SIZE	0x00180000

extern void crcut(char *s);
extern char *sp_pass(char *p);
extern void show_version(char *);
extern char *calc_md5_from_fp(FILE *, size_t);
extern char *calc_md5_from_file(char *);
extern char *MD5Digest(char *);

/* End */
