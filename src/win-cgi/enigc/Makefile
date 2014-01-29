CROSS_COMPILE	= i386-mingw32-

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip

#CFLAGS		= -Wall -I../include -I../.. -D_DEBUG
CFLAGS		= -Wall -I../include -I../..
CFLAGS		+= -DAI_VERSION=\"0.5.2\" -DENC

LDFLAGS		= --large-address-aware
LIBS		= -lwsock32

INSTDIR		= /AiRouter/Diskimage/RSM/src/Apache/htdocs/cgi-bin

OBJS1		= md5c.o md5ify.o md5calc.o pcnv.o mkrand.o
OBJS1		+= netclient.o showver.o winsocket.o
OBJS2		= enigc.o showver.o
OBJS3		= enirec.o showver.o
OBJS4		= upload.o showver.o

TARGET		= enigc.exe enigc.cgi enirec.cgi upload.cgi xdel.exe cat.exe

INSTTARGET	= $(INSTDIR)/enigc.cgi $(INSTDIR)/enigc.exe \
		  $(INSTDIR)/cat.exe $(INSTDIR)/xdel.exe \
		  $(INSTDIR)/enirec.cgi $(INSTDIR)/upload.cgi

SRCS		= showver.c md5c.c md5calc.c md5ify.c pcnv.c \
		  mkrand.c netclient.c winsocket.c enirec.c upload.c

all:	$(TARGET)

enigc.exe:	$(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LIBS)

cat.exe:	cat.c
	$(CC) $(CFLAGS) -o $@ $<

xdel.exe:	xdel.c
	$(CC) $(CFLAGS) -o $@ $<

enigc.cgi:	$(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $(OBJS2) $(LIBS)

enirec.cgi:	$(OBJS3)
	$(CC) -DENIREC $(LDFLAGS) -o $@ $(OBJS3) $(LIBS)

upload.cgi:	$(OBJS4)
	$(CC) -DUPLOAD $(LDFLAGS) -o $@ $(OBJS4) $(LIBS)

netclient.o:	netclient.c

showver.o:	showver.c

enirec.o:	enirec.c
	$(CC) $(CFLAGS) -DENIREC -c -o $@ $<

upload.o:	upload.c
	$(CC) $(CFLAGS) -DUPLOAD -c -o $@ $<

src:
	ln -fs ../../showver.c
	ln -fs ../../md5c.c
	ln -fs ../../md5calc.c
	ln -fs ../../md5ify.c
	ln -fs ../../pcnv.c
	ln -fs ../../mkrand.c
	ln -fs ../../netclient.c
	ln -fs ../../winsocket.c
	ln -fs enigc.c enirec.c
	ln -fs enigc.c upload.c

$(INSTDIR)/enigc.exe:	enigc.exe
	install -c -m 0755 $< $(INSTDIR)

$(INSTDIR)/cat.exe:	cat.exe
	install -c -m 0755 $< $(INSTDIR)

$(INSTDIR)/xdel.exe:	xdel.exe
	install -c -m 0755 $< $(INSTDIR)

$(INSTDIR)/enigc.cgi:	enigc.cgi
	install -c -m 0755 $< $(INSTDIR)

$(INSTDIR)/enirec.cgi:	enirec.cgi
	install -c -m 0755 $< $(INSTDIR)

$(INSTDIR)/upload.cgi:	upload.cgi
	install -c -m 0755 $< $(INSTDIR)

install: $(INSTTARGET)
##	@for i in $(INSTTARGET) ; do \
##		echo $(STRIP) $$i ; \
##		$(STRIP) $$i ; \
##	done
	@touch install

clean:
	rm -f *.o *.cgi *.exe 

cleansrc:
	rm -f $(SRCS)
