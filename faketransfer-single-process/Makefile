CC=gcc
CFLAGS=-O2
OBJONE=faketransfer-cli.c fakeupload.c fakedownload.c fakecommon.c
OBJTWO=faketransfer-svr.c fakeupload.c fakedownload.c fakecommon.c

ALL:faketransfer-cli faketransfer-svr

faketransfer-cli:$(OBJONE)
	$(CC) $(CFLAGS) $(OBJONE) -o faketransfer-cli

faketransfer-svr:$(OBJTWO)
	$(CC) $(CFLAGS) $(OBJTWO) -o faketransfer-svr

.PHONY:clean

clean:
	rm faketransfer-cli faketransfer-svr
