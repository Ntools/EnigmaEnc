//******************************************************************
//	HS C COMMON LIBRARY SOURCE
//		MAKE DATE 2001/01/01	by HS	Ver1.00
//		EDIT DATE 2002/05/03	by HS	Ver1.01
//		EDIT DATE 2004/03/23	by HS	Ver1.58
//		EDIT DATE 2004/09/21	by HS	Ver1.60
//******************************************************************

//
//	Include Define
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "public.h"


#ifdef LINUX
#include <strings.h>
#include <syslog.h>	//	for use syslog()
#include <unistd.h>
#endif

#ifdef WIN32
#include <stdarg.h>
#endif

//
//	DEBUG
//
//#define _DEBUG_

#ifdef __SIDISK__
FILE *rw_fp = NULL;
#endif

FILE *RW_fopen(const char *path, const char *mode)
{
	FILE *fp;

	fp = fopen(path, mode);
#ifdef __SIDISK__
	if (fp != NULL && rw_fp == NULL) {
		if (strchr(mode, 'w') != NULL) {
#if 0 // don't change remount. neko.
			system("mount -o rw,remount /");
			usleep(30000);
#endif
			rw_fp = fp;
		}
	}
#endif

	return (fp);
}

int RO_fclose(FILE *fp)
{
	int s;

#ifdef __SIDISK__
	if (fp == rw_fp && rw_fp != NULL) {
#if 0 // don't remount. neko.
		usleep(30000);

		system("sync");
		system("sync");
		system("mount -o ro,remount /");
#endif
		rw_fp = NULL;
	}
#endif
	s = fclose(fp);
	return s;
}
//******************************************************************
//	Config File Read
//	INPUT	lpcFileName	=	CONFIG File Path
//		lpcKey		=	Key Name String
//	OUTPUT	lpcValue	=	Data
//	RETURN	[0:OK,else:NG]
//******************************************************************
int com_getConfig(char *lpcFileName,char *lpcKey, char *lpcValue)
{
	//
	//	value define
	//
	FILE	*fp;
	char	strBuf[1024];
	char	*lpP;

	//
	//	File Access
	//
	fp = RW_fopen(lpcFileName,"r");
	if( fp == NULL )
	{
#ifdef _DEBUG_
		printf("file open error.%s", lpcFileName);
#endif
		return 1;
	}

	//
	//	Read
	//
	lpcValue[0] = 0x00;
	while( ( fgets( strBuf, sizeof(strBuf), fp ) != NULL ) )
	{
		lpP = (char *)strtok(strBuf,"\r\n\t ");
		if( lpP != NULL ) 
		{
			if( strlen(lpP) == 0 )
			{
				while( NULL != (lpP = (char *)strtok(NULL,"\r\n\t ") ))
				{
					if( strlen(lpP) != 0 ) break;
				}
			}
			if( strcmp(lpP, lpcKey) == 0 )
			{
				//	Data Split
				com_purgeStr(&lpP[strlen(lpcKey)+1],lpcValue);
				////printf("<br>%s->%s<br>\n",lpcKey,lpcValue);
				break;
			}
		}
	}
	fclose(fp);

	//
	//	Ret Value
	//
	return 0;
}

//******************************************************************
//	Config File Read Equal
//	INPUT	lpcFileName	=	CONFIG File Path
//		lpcKey		=	Key Name String
//	OUTPUT	lpcValue	=	Data
//	RETURN	[0:OK,else:NG]
//******************************************************************
int com_getConfigEq(char *lpcFileName,char *lpcKey, char *lpcValue)
{
	//
	//	value define
	//
	int     i;
	FILE	*fp;
	char	strBuf[1024];
	char	*lpP;
#ifdef _DEBUG_
	int bDebug = 0;
#endif

	//
	//	File Access
	//
	fp = RW_fopen(lpcFileName,"r");
	if( fp == NULL )
	{
#ifdef _DEBUG_
		printf("file open error.%s", lpcFileName);
#endif
		return 1;
	}

#ifdef _DEBUG_
	syslog(LOG_DEBUG, "com_getConfigEq(%s, %s, %s);", lpcFileName, lpcKey, lpcValue);
	if(strcmp(lpcKey, "security_inf0") == 0){
		bDebug = 1;
	}
#endif

	//
	//	Read
	//
	lpcValue[0] = 0x00;
	while( ( fgets( strBuf, sizeof(strBuf), fp ) != NULL ) )
	{
#ifdef _DEBUG_
		if(bDebug)
			syslog(LOG_DEBUG, "%s", strBuf);
#endif
//		StrReplace( strBuf, "=", " " );
		for(i=0; strBuf[i] != '\0'; i++){
			if(strBuf[i] == '='){
				strBuf[i] = ' ';
				break;
			}
		}
#ifdef _DEBUG_
		if(bDebug)
			syslog(LOG_DEBUG, "%s", strBuf);
#endif
		lpP = (char *)strtok(strBuf,"\r\n\t ");
		if( lpP != NULL ) 
		{
			if( strlen(lpP) == 0 )
			{
				while( NULL != (lpP = (char *)strtok(NULL,"\r\n\t ") ))
				{
					if( strlen(lpP) != 0 ) break;
				}
			}
			if( strcmp(lpP, lpcKey) == 0 )
			{
				//	Data Split
#ifdef _DEBUG_
				if(bDebug){
					syslog(LOG_DEBUG, "DataSplit");
				}
#endif
				com_purgeStr(&lpP[strlen(lpcKey)+1],lpcValue);
#ifdef _DEBUG_
				if(bDebug){
					syslog(LOG_DEBUG, "%s, %s", strBuf, lpcValue);
				}
#endif
				////printf("<br>%s->%s<br>\n",lpcKey,lpcValue);
				break;
			}
		}
	}
	fclose(fp);

	//
	//	Ret Value
	//
	return 0;
}

//******************************************************************
//	HTML FILE READ OUT
//	INPUT	lpcFileName	=	HTML File Name
//		lpcValuPath	=	DATA File Name
//		iAdmin		=	[1:admin,2:user]
//	OUTPUT	(none)
//	RETURN	[0:OK,else:NG]
//******************************************************************
int com_HtmlDisp(char *lpcFileName,char *lpcValuPath, int iAdmin,int no,char *logpath)
{
	//
	//	value define
	//
	int	iRet=0;
	FILE	*fp;
	char	strBuf[4096];
	char	strNo[10];
        char    strExecDir[192];
        char    strExecPath[192];
	FILE    *fp2;
	char    buf[128];
	char	strVersion[32];
	char	strPortMessage[256];

#ifdef _DEBUG_
	syslog(LOG_DEBUG, "com_HtmlDisp(%s, %s, %d, %d, %s)", lpcFileName, lpcValuPath, iAdmin, no, logpath);
#endif

	//
	//	get version
	//
	fp = RW_fopen(VERSION_PATH, "r");
	if(fp){
		fgets(strVersion, 31, fp);
		fclose(fp);
		syslog(LOG_DEBUG, "version:%s filename:%s", strVersion, VERSION_PATH);
	}else{
		syslog(LOG_DEBUG, "version:open error");
		strcpy(strVersion, "version error");
	}

#ifdef LINUX
	fp = RW_fopen("/var/spool/nkycgi/secu_level_inf.conf", "r");
	if(fp){
		while(fgets(strBuf, sizeof(strBuf), fp)!=NULL){
			if( strstr(strBuf, "secu_level_r1=checked") != NULL )
				strcpy(strPortMessage, SECURITY_LEVEL1_MESSAGE);
			if( strstr(strBuf, "secu_level_r2=checked") != NULL )
				strcpy(strPortMessage, SECURITY_LEVEL2_MESSAGE);
			if( strstr(strBuf, "secu_level_r3=checked") != NULL )
				strcpy(strPortMessage, SECURITY_LEVEL3_MESSAGE);
			if( strstr(strBuf, "secu_level_r4=checked") != NULL )
				strcpy(strPortMessage, SECURITY_LEVEL4_MESSAGE);
		}
		fclose(fp);
	}
#endif


	//
	//	File Access
	//
	fp = RW_fopen(lpcFileName,"r");
	if( fp != NULL ) {
		//	read File
		while( ( fgets( strBuf, sizeof(strBuf), fp ) != NULL ) ) {
			//	USER MODE
			if( iAdmin == 1 ) {
				StrReplace( strBuf, "<% users_start $>", " -->");
				StrReplace( strBuf, "<% users_end $>", "<!-- ");
			// Linux USER MODE
#ifdef LINUX
				StrReplace( strBuf, "<% linux_users_start $>", " -->");
				StrReplace( strBuf, "<% linux_users_end $>", "<!-- ");
#endif
			// Windows USER MODE
#ifdef WIN32
				StrReplace( strBuf, "<% windows_users_start $>", " -->");
				StrReplace( strBuf, "<% windows_users_end $>", "<!-- ");
#endif
			}
			// Linux MODE
#ifdef LINUX
				StrReplace( strBuf, "<% linux_start $>", " -->");
				StrReplace( strBuf, "<% linux_end $>", "<!-- ");
#endif
			// Windows MODE
#ifdef WIN32
				StrReplace( strBuf, "<% windows_start $>", " -->");
				StrReplace( strBuf, "<% windows_end $>", "<!-- ");
#endif
			//	SESSION ID
			sprintf(strNo,"%d",no);
			StrReplace( strBuf, "<%no%>",strNo);
			//	logpath
			StrReplace( strBuf, "<%logpath%>",logpath);
			//	version
			StrReplace( strBuf, "<%version%>",strVersion);
			//	open port message.
			StrReplace( strBuf, "<%port_message%>", strPortMessage);
			//	system status
			if(strstr( strBuf, "<%system_status%>")){
				com_getConfigEq(DEF_CONFPATH,"execdir",strExecDir);
				sprintf(strExecPath,"%s%s",strExecDir,"print_if_status.sh");


				fp2 = fopen("/tmp/pppoe_status.txt", "r");
				if(fp2){

					while(fgets(buf, 128, fp2)){
						puts(buf);
					}

					fclose(fp2);
				}

				//system(strExecPath);
#ifdef _DEBUG_
				syslog(LOG_DEBUG, "*** %s ***",strExecPath);
#endif
			}
			else{
				//	DATA
				com_ReplaceHtmlDisp(strBuf,lpcValuPath);
			}
		}
		fclose(fp);
	} else {
		//	error
		iRet = 1;
		//
		// DEBUG
		//
#ifdef _DEBUG_
		if(lpcFileName != NULL)
			printf("[%s] load error.<br>", lpcFileName);
#endif
	}

	//
	//	Ret Value
	//
	return iRet;
}

//******************************************************************
//	HTML FILE READ OUT
//	INPUT	lpcLine		=	HTML Data
//              lpcValuPath     =       DATA File Name
//	OUTPUT	(none)
//	RETURN	[0:OK,else:NG]
//******************************************************************
void com_ReplaceHtmlDisp(char *lpcLine, char *lpcValuPath)
{
        char lookup[64];
        int  p1,p2;
        char *nowp;
	char lpcKeyName[64];
	char lpcData[144];

	char lpcKeyNameIp[64];
	char *po,*topp;
	char buf[16] = {0};

        nowp = lpcLine;
        while( NULL != (nowp = com_lookedStr(lpcLine, "<%","%>",lookup,&p1,&p2)) )
	{
		//
		strcpy(lpcKeyName,lookup);
		StrReplace( lpcKeyName, "<%", "");
		StrReplace( lpcKeyName, "%>", "");
	        memset(lpcData,0x00,sizeof(lpcData));
		
		if( strstr(lpcKeyName,"_IP1") != NULL ) {
			strcpy(lpcKeyNameIp,lpcKeyName);
			StrReplace( lpcKeyNameIp,"_IP1","");
			com_getConfigEq(lpcValuPath,lpcKeyNameIp,lpcData);
                	if( (po=strstr(lpcData,".")) != NULL ) {
				po[0] = 0x00;
				StrReplace( lpcLine, lookup, lpcData);
			} else {
				StrReplace( lpcLine, lookup, "");
			}
		} else if( strstr(lpcKeyName,"_IP2") != NULL ) {
			strcpy(lpcKeyNameIp,lpcKeyName);
			StrReplace( lpcKeyNameIp,"_IP2","");
			com_getConfigEq(lpcValuPath,lpcKeyNameIp,lpcData);
                	if( (po=strstr(lpcData,".")) != NULL ) {
				 po++; topp = po;
                		if( (po=strstr(po,".")) != NULL ) {
					po[0] = 0x00;
					StrReplace( lpcLine, lookup, topp);
				} else {
					StrReplace( lpcLine, lookup, "");
				}
			} else {
				StrReplace( lpcLine, lookup, "");
			}
		} else if( strstr(lpcKeyName,"_IP3") != NULL ) {
			strcpy(lpcKeyNameIp,lpcKeyName);
			StrReplace( lpcKeyNameIp,"_IP3","");
			com_getConfigEq(lpcValuPath,lpcKeyNameIp,lpcData);
                	if( (po=strstr(lpcData,".")) != NULL ) { 
				po++; topp = po;
                		if( (po=strstr(po,".")) != NULL ) {
					 po++; topp = po;
					if( (po=strstr(po,".")) != NULL ) {
						po[0] = 0x00;
						StrReplace( lpcLine, lookup, topp);
					} else { 
						StrReplace( lpcLine, lookup, "");
					}
				} else { 
					StrReplace( lpcLine, lookup, "");
				}
			} else { 
				StrReplace( lpcLine, lookup, "");
			}
		} else if( strstr(lpcKeyName,"_IP4") != NULL ) {
			strcpy(lpcKeyNameIp,lpcKeyName);
			StrReplace( lpcKeyNameIp,"_IP4","");
			com_getConfigEq(lpcValuPath,lpcKeyNameIp,lpcData);
                	if( (po=strstr(lpcData,".")) != NULL ) {
				po++; topp = po;
				if( (po=strstr(po,".")) != NULL ) {
					 po++; topp = po;
		 			if( (po=strstr(po,".")) != NULL ) {
						po++; topp = po;
						// dhcp support.
						if(strstr(lpcKeyName,"dhcp_end_IP4")){
							sprintf(buf, "%d", atoi(topp)-1);
							StrReplace( lpcLine, lookup, buf );
						} else {
							StrReplace( lpcLine, lookup, topp);
						}
					} else {
						StrReplace( lpcLine, lookup, "");
					}
				} else {
					StrReplace( lpcLine, lookup, "");
				}
			} else {
				StrReplace( lpcLine, lookup, "");
			}
		} else {	
			com_getConfigEq(lpcValuPath,lpcKeyName,lpcData);
                	StrReplace( lpcLine, lookup, lpcData);
		}
	}
	printf("%s",lpcLine);
}

//******************************************************************
//	The extraction of the character
//	INPUT	*src		=	source string
//	OUTPUT	*dst		=	dist string
//	RETURN	(none)
//******************************************************************
void com_purgeStr(char *src, char *dst)
{
	int i;
	for( i=0; i<strlen(src); i++ )
	{
		if( src[i] != ' ' && src[i] != '\t' && src[i] != '\n' ) 
		{
			break;
		}
	}
	if(src[i] == '\"')
		i++;
	strcpy(dst,&src[i]);
	for( i=0; i<strlen(dst); i++ )
	{
		if( dst[i] == ' ' || dst[i] == '\t' || dst[i] == '\n' || dst[i] == '\"' )
		{
			dst[i] = 0x00;
			break;
		}	
	}
}

//******************************************************************
//	The character put in the designated string is looked up.
//	INPUT	*src		=	source string
//		*split1		=	Token Start Strings 1
//		*split2		=	Token End Strings 2
//	OUTPUT	*lookstr	=	looked up
//		*p1		=	start pointer
//		*p2		=	end pointer
//	RETURN	Next Pointer	=	[NULL=END]
//******************************************************************
char *com_lookedStr(char *src, char *split1, char *split2, char *lookstr, int *p1, int *p2)
{
	char *edp;

	//
	//
	//
	char *stp = strstr(src,split1);
	if( stp == NULL )
	{
		*p1 = -1;
		*p2 = -1;
		lookstr[0] = 0x00;
		return NULL;
	}

	//
	//
	//
	edp = strstr(stp,split2);
	if( edp == NULL )
	{
		*p1 = -1;
		*p2 = -1;
		lookstr[0] = 0x00;
		return NULL;
	}

	//
	//
	//
	*p1 = (int)(stp - src);
	*p2 = (int)(edp - src);
	lookstr[0] = 0x00;
	strncpy(lookstr,stp,(*p2+strlen(split2)-*p1));
	lookstr[(*p2+strlen(split2)-*p1)] = 0x00;

	//
	//
	//
	return (char *)(edp + strlen(split2));
}

//*********************************************************
//
//*********************************************************
char *StrShift( char *String, size_t nShift )
{
	char *start = String;
	char *stop  = String + strlen( String );
	memmove( start + nShift, start, stop-start+1 );

	return String + nShift;
}

//*********************************************************
//
//*********************************************************
char *StrReplace( char *String, const char *From, const char *To )
{
	int   nToLen;
	int   nFromLen;
	int   nShift;
	char *start;
	char *stop;
	char *p;

	nToLen   = strlen( To );
	nFromLen = strlen( From );
	nShift   = nToLen - nFromLen;
	start    = String;
	stop     = String + strlen( String );

	//
	while( NULL != ( p = strstr( start, From ) ) )
	{
		//
		start = StrShift( p + nFromLen, nShift );
		stop  = stop + nShift;

		//
		memmove( p, To, nToLen );
	}

	return String;
}

//*********************************************************
//
//*********************************************************
int com_urlSplit( char *src, char **name, char **value )
{
	int	i,nCur;	
	int count = 0;
	nCur = 0;
	if( src[0] == 0x00){
		return 0;
	}
	name[0] = src;
	for( i = 0 ; src[i] != '\0' ; ++i ){

		if ( src[i] == '='){
			if( count < 1 ){
				src[i] = '\0';
				value[nCur] = src + i + 1;
				count++;
			}
		}
		if( src[i] == '&' ){
			src[i] = '\0';
			count = 0;
			nCur++;
			name[nCur] = src + i +1;
		}
	}
	return (nCur+1);
}

//*********************************************************
//
//*********************************************************
void com_urlDecode(char *src)
{
	int	i,j;
	int	length;
	char	*dst;
	char	calc;

	length = strlen(src);

	dst = (char *)malloc( length );
	for ( i=0,j=0;
		i < length ;
		i++, j++ ) {
		if ( src[i] == '+' ) {
			dst[j] = ' ';
		} else if ( src[i] == '%' ){
			i++ ;
			if ( src[i] >= 'A'){
				calc = src[i] - 'A' + 10;
			} else {
				calc = src[i] - '0';
			}
			calc = calc << 4;
			i++ ;
			if ( src[i] >= 'A'){
				calc += src[i] - 'A' + 10;
			} else {
				calc += src[i] - '0';
			}
			dst[j] = calc;
		} else {
			dst[j] = src[i];
		}
	}
	dst[j] = 0x00;
	strcpy( src, dst );
	free(dst);
}

//******************************************************************
//	NOMALIZE IP ADDRESS RANGE
//	INPUT	ip		=	ip address
//	OUTPUT	ip
//******************************************************************
int com_nomalizeIP(int ip)
{
	if(ip < 0) ip = 0;
	if(ip > 255) ip = 255;

	return ip;
}



//******************************************************************
//	HTML DATA FILE WRITE
//	INPUT	lpcLine		=	HTML Data
//              lpcValuPath     =       DATA File Name
//	OUTPUT	(none)
//	RETURN	[0:OK,else:NG]
//******************************************************************
int com_repFileData(char *lpcFilename, char *name[], char *value[], int iFields)
{
	FILE	*fp;
	int	i;
	char	strWork[1024];
	char	*lpP;
	int	ip1 = 0,ip2 = 0,ip3 = 0,ip4 = 0;
	char	namebuf[256];
	int	portno;
	int	pkg_no;

	fp = RW_fopen(lpcFilename,"w");
	if( fp == NULL ) {
#ifdef _DEBUG_
		//
		// DEBUG
		//
		lpcFilename == NULL ? 
			printf("com_repFileData() file open error.") :
			printf("%s file open error.", lpcFilename); 
#endif
		//	error
		return 1;
	}

	for( i=0; i<iFields; i++ )
	{
#ifdef _DEBUG_
		syslog(LOG_DEBUG, "name=%s, value=%s", name[i], value[i]);
#endif
		if( strstr(name[i],"_IP1") != NULL ) {
			ip1 = atoi(value[i]);
		} else if( strstr(name[i],"_IP2") != NULL ) {
			ip2 = atoi(value[i]);
		} else if( strstr(name[i],"_IP3") != NULL ) {
			ip3 = atoi(value[i]);
		} else if( strstr(name[i],"_IP4") != NULL ) {
			ip4 = atoi(value[i]);
			strcpy(namebuf,name[i]);
			StrReplace( namebuf, "_IP4", "");
			if( ip1 == 0 ) {
				fprintf(fp,"%s=\n",namebuf);
			} else {
				// dhcp support.
				if(strstr( name[i], "dhcp_end" )){
					ip4++;
				}
				ip1 = com_nomalizeIP(ip1);
				ip2 = com_nomalizeIP(ip2);
				ip3 = com_nomalizeIP(ip3);
				ip4 = com_nomalizeIP(ip4);
				fprintf(fp,"%s=%d.%d.%d.%d\n",namebuf,ip1,ip2,ip3,ip4);
			}
		}
		else if( strstr(name[i], "portfw_inp") != NULL ) {
			//	port forwarding
			portno = atoi(value[i]);
			if(portno < 0) portno = 0;
			if(portno > 65535) portno = 65535;
			fprintf(fp, "%s=%d\n",name[i], portno );
		}
		else if( strstr(value[i],"_checked") != NULL ) {
			//	RADIO
			memset(strWork,0x00,sizeof(strWork));
			strcpy(strWork,value[i]);
			lpP = strstr(strWork,"_checked");
			*lpP = 0x00;
			fprintf(fp,"%s=%s\n",strWork,"checked");
		}
		else if( strstr(value[i],"_selected") != NULL )
		{
			//	SELECT
			memset(strWork,0x00,sizeof(strWork));
			strcpy(strWork,value[i]);
			lpP = strstr(strWork,"_selected");
			*lpP = 0x00;
			fprintf(fp,"%s=%s\n",strWork,"selected");
		}
		else if( strstr( name[i], "prctl_pkg" ) != NULL )
		{
			//	prctl_pkg
			pkg_no = atoi(value[i]);
			if(pkg_no < 0) pkg_no = 0;
			if(pkg_no > 99) pkg_no = 99;
			fprintf(fp, "%s=%d\n",name[i], pkg_no);
		}
		else if( strstr(name[i],"presetvalue") != NULL )
		{
			//	Preset value
			fprintf(fp, "%s=presetvalue\n",name[i]);
		}
                else if( strstr(name[i],"firm_update") != NULL )
                {
                        //      firm update
                        fprintf(fp, "%s=firm_update\n",name[i]);
                }
		else if( strstr(name[i],"Reboot") != NULL )
		{
			//	Reboot
			fprintf(fp, "%s=Reboot\n",name[i]);
		}
		else if( strstr(name[i],"ping_exec") != NULL )
		{
			//	pinging
			fprintf(fp, "%s=ping_exec\n",name[i]);
		}
		else if( strstr(name[i],"traceroute_exec") != NULL )
		{
			//	trace routing
			fprintf(fp, "%s=traceroute_exec\n",name[i]);
		}
		else if( strstr(name[i],"Update") != NULL )
		{
			//	Update
			fprintf(fp, "%s=Update\n",name[i]);
		}
		else
		{
			//	NORMAL
			fprintf(fp,"%s=\"%s\"\n",name[i],value[i]);
		}	
	}
	fclose(fp);

	//
	//	Ret Value
	//
	return 0;
}

//******************************************************************
//	SET COOKIE
//	INPUT	(none)
//	OUTPUT	(none)
//	RETURN	(none)
//******************************************************************
void com_setCookie(char *admin)
{
	char tmpbuf[128];
	struct tm expires = { 0,0,12,25,11,101};
	
	strftime( tmpbuf, 128,"%a %d-%b-%Y %H:%M:%S",&expires );
	 
	//printf("Set-Cookie: name=router; EXPIRES=%s\n\n", tmpbuf );
	printf("Set-Cookie: name=%s; path=/\n\n",admin );

}

//******************************************************************
//	GET COOKIE
//	INPUT	(none)
//	OUTPUT	(none)
//	RETURN	(none)
//******************************************************************
int com_getCookie(void)
{
	int	nFields;
	int	nLength;
	char	*cookie;
	char	*name[1];
	char	*value[1];
	int	iRet = 0;

	if( getenv("HTTP_COOKIE") != NULL ) {
		nLength = strlen(getenv("HTTP_COOKIE"));
		cookie = (char *)malloc( nLength +1 );
		strcpy(cookie,getenv("HTTP_COOKIE"));
		com_urlDecode(cookie);
		nFields = com_urlSplit( cookie, name, value );
		if( strcmp(value[0],"admin") == 0 ) {
			iRet=1;
#ifdef _DEBUG_
			printf("getCookie = admin<br>");
#endif
		}
		if( strcmp(value[0],"user") == 0 ) {

			iRet=2;
#ifdef _DEBUG_
			printf("getCookie = user<br>");
#endif
		}
	} else {
#ifdef _DEBUG_
		printf("Cookie has no value<br>");
#endif

		nLength = 0;
		iRet=0;
	}

	//
	//
	//
	return iRet;
}


//******************************************************************
//	LOGIN CHECK
//	INPUT	int  no			=	SESSION ID
//		char *uid,*ups		=	USER ID,PASS
//		char *sid,*sps		=	ADMIN ID,PASS
//		char *lpcFileName	=	SESSION FILE PATH
//		int  *iAdmin		=	[1:Admin,2:User]
//	OUTPUT	(none)
//	RETURN	[0:Not Login,SESSION ID:Login OK]
//******************************************************************
int com_loginCheck(int no,char *uid,char *ups,char *sid,char *sps,char *lpcFileName,int *iAdmin)
{
	FILE	*fp;
	long	l_rdate;
	int	c_rno;
	char	c_rip[15];
	char	c_ruser_id[255];
	char	c_rpassword[255];
	char	c_ip[24] = {0};
	time_t	ltime = 0;
	char    buf[256] = {0};

	//
	//	FILE READ
	//
	fp = RW_fopen(lpcFileName,"r");
	if( fp == NULL ) {
		//	Error Not File
#ifdef _DEBUG_
                printf("file open error.%s", lpcFileName);
#endif
		return -1;
	}
	fscanf(fp,"%ld %d %s %s %s",(long *)&l_rdate,&c_rno,c_rip,c_ruser_id,c_rpassword);
	fclose(fp);
	
	//
	//  file write
	//
	fp = RW_fopen(lpcFileName,"w");
	if( fp == NULL ) {
#ifdef _DEBUG_
                printf("file write error.%s", lpcFileName);
#endif
		return -2;
	}
	fprintf(fp,"%ld %d %s %s %s\n",(long)ltime,no,c_rip,c_ruser_id,c_rpassword);
	fclose(fp);

	//
	//	No. CHECK
	//
//	if(no == c_rno){
//		if(no <= 999){
//			no++;
//		}else{
//			no=1;
//		}
//	}else{
//		printf("no:%d == c_rno:%d<br>", no, c_rno);
//		return -3;
//	}
	
	//
	//  ip check
	//
#ifdef LINUX
	strncpy(c_ip,getenv("REMOTE_ADDR"), 22);
#ifdef _DEBUG_
	if( strcmp(c_ip,c_rip) != 0 ) {
		return -4;
	}
#endif // _DEBUG_
#endif // LINUX

	//
	//  u id password check
	//
	*iAdmin = 1;
	if( (strcmp(uid,c_ruser_id) == 0 && strcmp(ups,c_rpassword) == 0) ) {
		*iAdmin = 2;
	} else if( (strcmp(sid,c_ruser_id) == 0 && strcmp(sps,c_rpassword) == 0) ) {
		*iAdmin = 1;
	} else {
		return -5;
	}

	//
	//  date check
	//
	time( &ltime );
	if((ltime - l_rdate) > 600){
		// time over
		strcpy(buf, ctime(&ltime));
//		printf("ltime = %d(%s), l_rdate = %d(%s)<br>", (int)ltime, buf, (int)l_rdate, ctime(&l_rdate));
		return -6;
	}

	//
	//  file write
	//
	fp = RW_fopen(lpcFileName,"w");
	if( fp == NULL ) {
		return -7;
	}
	fprintf(fp,"%ld %d %s %s %s\n",(long)ltime,no,c_rip,c_ruser_id,c_rpassword);
	fclose(fp);

	//
	// exclusive check.
	//
	fp = RW_fopen(EXCLUSIVE_FILE_NAME, "r");
	if(fp){
		fclose(fp);
		return -8;
	}



	//
	//	RETURN VALUE
	//
#if 1	/* ohkuma change */
	return 1;
#else
	return no;
#endif
}


//******************************************************************
//	LOGIN SET
//	INPUT	char *uid,*ups		=	USER ID,PASS
//		char *lpcFileName	=	SESSION FILE PATH
//	OUTPUT	(none)
//	RETURN	[0:NG,1:OK]
//******************************************************************
int com_loginSet(char *uid,char *ups,char *lpcFileName)
{
	FILE	*fp;
	char	c_ip[15];
	time_t	ltime;

	//
	//  ip
	//
	strcpy(c_ip,getenv("REMOTE_ADDR"));

	//
	//  date
	//
	time( &ltime );

	//
	//  file write
	//
	fp = RW_fopen(lpcFileName,"w");

	if( fp == NULL ) {
		//return 0;
		// ファイルがオープンできなかったら、正常終了にしてみる
		return 1;

	}
	fprintf(fp,"%ld %d %s %s %s\n",(long)ltime,1,c_ip,uid,ups);

	fclose(fp);

	//
	//	RETURN VALUE
	//
	return 1;
}

#ifdef WIN32
#define LOG_DEBUG 0
int syslog(int i, char* fmt, ...){

	FILE *fp;
	int result;

	fp = fopen("messages.txt", "a");

	if(fp){

		va_list args;
		va_start(args, fmt);
		result = vfprintf(fp, fmt, args);
		va_end(args);

		fprintf(fp, "\n");

		fclose(fp);
	}


	return result;

}

char* basename(char* path){
	static char filename[_MAX_FNAME];
	_splitpath(path,NULL,NULL,filename,NULL);
	return filename;
}
#endif
