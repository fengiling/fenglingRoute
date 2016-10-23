#

CC=gcc
PROC=proc

VPATH=${HOME}/soft/fenglingRoute:${OUTDIR}

#基路径
BASEDIR=${HOME}/soft/fenglingRoute

#临时编译文件路径
OUTDIR=${HOME}/out

#头文件搜索路径
INCL=-I${BASEDIR}/include

#链接库及路径
LIB_PATH=-L${HOME}/lib
#LIBS=-lc -lm -lstock -lfix -lmfs -lsop -ldbo20 -lstruct -lclntsh -lclntst11 -lsql11 -lncrypt11 -lclient11  -lcommon11 -lgeneric11 -lnls11 -lcore11  -lzdrzqs -lzdrz_database
#HSLIBS=-lbsso20 -lbssp20

LINK_FLAG=-I.  -fPIC  -g -o ${HOME}/soft/fenglingRoute/$@ $? ${INCL} ${LIB_PATH} #${LIBS} ${HSLIBS}

.SUFFIXES:
.SUFFIXES: .pc .c .o 

.c.o:
	${CC} -I.  -fPIC -g -o $*.o  $< -c ${INCL}

all: fenglingRoute
#all: IFTSrz_20005 IFTSrz_20016 IFTSrz_20056 IFTSrz_20012 IFTSrz_20901 IFTSrz_20913 IFTSrz_20031 clean putbin

fenglingRoute:fenglingRoute.o routeServer.o routeClient.o routeConfig.o routeAllot.o
	$(CC) ${LINK_FLAG}

IFTSrz_3elide : iftsrz_3elide.o
	$(CC) ${LINK_FLAG}
	-rm -f  *.o
	-rm -f  ./src/*.lis
	-rm -f  iftsrz_3elide.c

clean:
	-rm -f  *.o

putbin:
	./putbin.sh