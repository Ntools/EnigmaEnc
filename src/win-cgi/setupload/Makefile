CROSS_COMPILE	= i386-mingw32-

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip

CFLAGS		= -Wall -I../include -D_DEBUG
CFLAGS		+= -DAI_VERSION=\"0.3.1\" -DENC

LDFLAGS		=
LIBS		=

INSTDIR		= /AiRouter/Diskimage/RSM/src/Apache/htdocs/cgi-bin

OBJS		= setupload.o showver.o

TARGET		= setupload.cgi

INSTTARGET	= $(INSTDIR)/setupload.cgi

all:	$(TARGET)

setupload.cgi:	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

setupload.o:	setupload.c

showver.o:	showver.c

src:
	ln -fs ../../showver.c

$(INSTDIR)/setupload.cgi:	setupload.cgi
	install -c -m 0755 $< $(INSTDIR)

install: $(INSTTARGET)
#	@for i in $(INSTTARGET) ; do \
#		echo $(STRIP) $$i ; \
#		$(STRIP) $$i ; \
#	done
	@touch install

clean:
	rm -f *.o $(TARGET)
