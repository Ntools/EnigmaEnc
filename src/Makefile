# Makefile 

CROSS_COMPILE	=

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip

#CFLAGS		= -Wall -O -DAI_VERSION=\"0.5.2\"
CFLAGS		= -g -D_DEBUG -Wall -O -DAI_VERSION=\"0.5.2\"

LDFLAGS		= -g
LIBS		= -lnsock -lpthread

OBJS		= md5c.o md5ify.o md5calc.o
OBJS		+= pcnv.o mkrand.o netsvr.o showver.o

OBJSC		= md5c.o md5ify.o md5calc.o
OBJSC		+= pcnv.o mkrand.o netclient.o showver.o

OBJT		= pcnv.o mkrand.o test.o

NETD		= enigd
NETC		= enigc
TARGET		= $(NETD) $(NETC)

INSTDIR		= /AiRouter/Diskimage/diskimage/_usr/sbin

INSTTARGET	= $(INSTDIR)/$(NETD)

.c.o:	$(CC) -c $(CFLAGS) $@

all: $(TARGET) test

test:	$(OBJT)
	$(CC) $(LDFLAGS) -o $@ $(OBJT) $(LIBS)

$(NETC):	$(OBJSC)
	$(CC) $(LDFLAGS) -o $@ $(OBJSC) $(LIBS)

$(NETD):	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

mkrand.o:	mkrand.c

pcnv.o:		pcnv.c

netsvr.o:	netsvr.c

showver.o:	showver.c

md5c.o:		md5c.c

md5ify.o:	md5ify.c

md5calc.o:	md5calc.c


install:	$(INSTTARGET)
	@touch install

$(INSTTARGET):	$(NETD)
	@for i in $(NETD) ; do \
		echo \
		install -c -m 0755 $$i $(INSTDIR) ; \
		install -c -m 0755 $$i $(INSTDIR) ; \
		strip $(INSTTARGET) ; \
	done

clean:
	rm -f *.o $(TARGET) test
