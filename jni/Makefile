# libsusrv.so
# Makefile for non Android debug build.
# 

SUSRV_IMPL_CFLAGS=-fPIC -c -Wall -g
SUSRV_SO_CFLAGS=-shared
SUSRV_SO_OBJS=su_srv.o su_shell_session.o su_srv_log.o su_srv_pfs.o 
SUSRV_SO_FILE=libsusrv.so


SUSRV_CLIENT_CFLAGS= -Wall -g
SUSRV_CLIENT_LFLAGS=-L. -lsusrv -pthread
SUSRV_CLIENT_FILE=susrv-client

libsusrv.so: $(SUSRV_SO_OBJS)
	gcc $(SUSRV_SO_CFLAGS) -o $(SUSRV_SO_FILE) $(SUSRV_SO_OBJS)

# note: add -DSU_SRV_TEST_BUILD to build for non Android context
su_srv.o: su_srv.c
	gcc $(SUSRV_IMPL_CFLAGS) -DSU_SRV_TEST_BUILD $<

su_shell_session.o: su_shell_session.c
	gcc $(SUSRV_IMPL_CFLAGS) $<

su_srv_log.o: su_srv_log.c
	gcc $(SUSRV_IMPL_CFLAGS) $<

su_srv_pfs.o: su_srv_pfs.c
	gcc $(SUSRV_IMPL_CFLAGS) $<

client: main.c
	gcc $(SUSRV_CLIENT_CFLAGS) -o $(SUSRV_CLIENT_FILE) $< $(SUSRV_CLIENT_LFLAGS)

clean:
	rm -f *.o
	rm -f $(SUSRV_SO_FILE)
	rm -f $(SUSRV_CLIENT_FILE)
	rm -f *~

all: clean libsusrv.so client

