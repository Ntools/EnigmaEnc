CROSS_COMPILE	= i386-mingw32-

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip

CFLAGS		= -Wall -I../include -I../.. -D_DEBUG
CFLAGS		+= -DAI_VERSION=\"0.3.1\"

LDFLAGS		=
LIBS		=

INSTDIR		= /AiRouter/Diskimage/RSM/src/Apache/htdocs/cgi-bin

OBJS		= showver.o md5c.o md5ify.o md5calc.o md5out.o

TARGET		= md5out.exe

INSTTARGET	= $(INSTDIR)/md5out.exe

all:	$(TARGET)

md5out.exe:	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

md5out.o:	md5out.c

src:
	ln -fs ../../showver.c
	ln -fs ../../md5c.c
	ln -fs ../../md5calc.c
	ln -fs ../../md5ify.c
	ln -fs ../../md5out.c

showver.o:	showver.c

$(INSTDIR)/md5out.exe:	md5out.exe
	install -c -m 0755 $< $(INSTDIR)

install: $(INSTTARGET)
	@for i in $(INSTTARGET) ; do \
		echo $(STRIP) $$i ; \
		$(STRIP) $$i ; \
	done
	@touch install

clean:
	rm -f *.o $(TARGET)
