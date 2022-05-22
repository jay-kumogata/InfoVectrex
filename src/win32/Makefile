CC = gcc
TARBALL = InfoVectrex_04J

# InfoVectrex
.CFILES =	../M6809.cpp \
		../Memory.cpp \
	       ../M6522.cpp \
		../VG.cpp \
		../SG.cpp \
		./System.cpp

.OFILES	=	$(.CFILES:.cpp=.o)

#CCFLAGS =  -mno-cygwin -DDEBUG
#CCFLAGS = -mno-cygwin
CCFLAGS =
#LDFILGS = -mno-cygwin -lstdc++ -lwinmm -lgdi32 -ldsound # -mwindows 
LDFILGS = -lstdc++ -lwinmm -lgdi32 -ldsound # -mwindows 

all: InfoVectrex

InfoVectrex: $(.OFILES)
	$(CC) $(INCLUDES) -o $@ $(.OFILES) $(LDFILGS) -lm -lz

.cpp.o:
	$(CC) $(INCLUDES) -c $(CCFLAGS) $*.cpp  -o $@

clean:
	rm -f $(.OFILES) ./*~ ../*~ *.stackdump

cleanall:
	rm -f $(.OFILES) ./*~ ../*~ *.stackdump InfoVectrex.exe

release: clean all

tar:
	( cd ..; \
	tar cvf $(TARBALL).tar ./*; \
	gzip $(TARBALL).tar \
	)

install:
	install ./InfoVectrex /usr/local/bin
