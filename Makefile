#

CC=gcc
PROC=proc

VPATH=${HOME}/soft/fenglingTunnel:${OUTDIR}

#基路径
BASEDIR=${HOME}/soft/fenglingTunnel

#临时编译文件路径
OUTDIR=${HOME}/out

#头文件搜索路径
INCL=-I${BASEDIR} -I${BASEDIR}/include -I/usr/include 

#链接库及路径
LIB_PATH=-L${HOME}/lib -L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/x86_64-linux-gnu/openssl-1.0.0/engines/ -L/lib/x86_64-linux-gnu
#LIBS=-lc -lm -lstock -lfix -lmfs -lsop -ldbo20 -lstruct -lclntsh -lclntst11 -lsql11 -lncrypt11 -lclient11  -lcommon11 -lgeneric11 -lnls11 -lcore11  -lzdrzqs -lzdrz_database
#HSLIBS=-lbsso20 -lbssp20
LIBS=-lssl -lcrypto

LINK_FLAG=-I.  -fPIC  -g -o ${HOME}/soft/fenglingTunnel/$@ $? ${INCL} ${LIB_PATH} ${LIBS} #${HSLIBS}

PUBOBJ= fenglingCA.o fenglingEpoll.o fenglingConf.o \
fenglingHash.o fenglingDict.o fenglingCA.o \
fenglingLog.o fenglingShm.o  fenglingSem.o \
fenglingHTML.o fenglingDaemon.o fenglingProcess.o

.SUFFIXES:
.SUFFIXES: .pc .c .o 

.c.o:
	${CC} -I.  -fPIC -g -o $*.o  $< -c ${INCL}

all: Cmain Smain fenglingShow clean
#all: fenglingRoute
#all: IFTSrz_20005 IFTSrz_20016 IFTSrz_20056 IFTSrz_20012 IFTSrz_20901 IFTSrz_20913 IFTSrz_20031 clean putbin

fenglingRoute:fenglingRoute.o routeServer.o routeClient.o routeConfig.o routeAllot.o
	$(CC) ${LINK_FLAG}

CAClient:CAClient.o
	$(CC) ${LINK_FLAG}
CAServer:CAServer.o
	$(CC) ${LINK_FLAG}

Cmain:Cmain.o  ${PUBOBJ}
	$(CC) ${LINK_FLAG}
	#-rm -f  *.o
	
Smain:Smain.o   ${PUBOBJ}
	$(CC) ${LINK_FLAG}
	#-rm -f  *.o
	
fenglingShow:fenglingShow.o  ${PUBOBJ}
	$(CC) ${LINK_FLAG}
	
clean:
	-rm -f  *.o

putbin:
	./putbin.sh
#
#
###################################################
#
#/*Makefile文件*/
#
#CC = $(CROSS_COMPILE)gcc
#
#CFLAGS = -Wall -g -D_DEBUG_
#
##.PHONY:all clean mqueue
#
#TARGETS = t1 t2
#
#all:$(TARGETS)
#
#t1: shm_producer.o 
# $(CC) -o $@ $^
#t2: shm_customer.o 
# $(CC) -o $@ $^
##main.o:main.c
#
##link.o:link.c
#
#clean:
# rm -f *~ *.o $(TARGETS)
##mqueue:
## $(MAKE) -C mqueue