CC=gcc
CFLAGS=-O2 -W -Werror -Wall
OBJONE=faketransfer-mt-cli.c faketransfer-mt-cli-fun.c
OBJTWO=faketransfer-mt-svr.c faketransfer-mt-svr-tpool.c faketransfer-mt-svr-fun.c

ALL:faketransfer-mt-cli faketransfer-mt-svr

faketransfer-mt-cli:$(OBJONE)
	$(CC) $(CFLAGS) $(OBJONE) -o faketransfer-mt-cli -lpthread

faketransfer-mt-svr:$(OBJTWO)
	$(CC) $(CFLAGS) $(OBJTWO) -o faketransfer-mt-svr -lpthread

.PHONY:clean

clean:
	rm faketransfer-mt-cli faketransfer-mt-svr
