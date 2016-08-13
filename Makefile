ALL:faketransfer-cli faketransfer-svr
OBJONE=faketransfer-cli.c faketransfer.c fakecommon.c
OBJTWO=faketransfer-svr.c faketransfer.c fakecommon.c
faketransfer-cli:$(OBJONE)
	gcc $(OBJONE) -o faketransfer-cli
faketransfer-svr:$(OBJTWO)
	gcc $(OBJTWO) -o faketransfer-svr
clean:
	rm faketransfer-cli faketransfer-svr
