/***************************************************************************
 * Copyright 2000-2004 Nobby Noboru Hirano
 * All rights reserved
 *
 * Warning !!!
 * Distribute unfreely,
 * This program sources can be used authorized user only !!
 *
 *
 * Nobby Noboru Hirano <nobby@ntools.net>
 ***************************************************************************/


SOCKET Ssocket(const char *host, const int port);
SOCKET Socket(const char *host, const int port);
SOCKET UdpSock(const int port);
SOCKET UdpAdrSet(int port, char *ip, struct sockaddr_in *adr);
size_t SockRead(const int sock, void *buf,int len);
size_t SockPuts(const int sock, char *buf);
char *SockGets(const int sock, char *buf, int len);
size_t SockWrite(const int sock, void *buf,int len);
size_t SockPrintf(const int sock, char *format, ...);
/*
 each functions check timeout tout msec.
 if timeout return -2;
 */
int SockReadTout(const int sock, void *buf,int len, int tout);
int SockWriteTout(const int sock, void *buf,int len, int tout);
int SockGetsTout(const int sock, char *buf, int len, int tout);
int SockPrintfTout(const int sock, int tout, char *format, ...);

int SockStatus(int sock,int ms, fd_set *fds);
int WSockStatus(int sock,int ms, fd_set *fds);

/* sleep until TCP connection on a socket */
SOCKET act_rec(int sock, char **ip);
