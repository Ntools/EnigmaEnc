CROSS_COMPILE	= i386-mingw32-

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
#

CFLAGS		= -Wall -I../include -I../.. -D_DEBUG
CFLAGS		+= -DAI_VERSION=\"0.5.2\" -DENC

LDFLAGS		=
INSTDIR		= /AiRouter/Diskimage/RSM/src/Apache/htdocs/cgi-bin

all:
	make -C setupload
	make -C enigc
	make -C md5

xdel.exe:	xdel.c
	$(CC) $(CFLAGS) -o xdel.exe xdel.c

src:
	make -C setupload src
	make -C enigc src
	make -C md5 src

install:
	make -C setupload install
	make -C enigc install
	make -C md5 install
	install -c -m 0755 xdel.exe $(INSTDIR)

clean:
	make -C setupload clean
	make -C enigc clean
	make -C md5 clean
	rm -f *.o *.exe
