NAME = i-score

CC_32 = /usr/bin/clang++ -arch i386
CC_64 = /usr/bin/clang++ -arch x86_64

#########################################

SRC32 = main.i386.o
SRC64 = main.i386.x64.o

#########################################

INCLUDES = -I../../../Foundation/library/includes
INCLUDES += -I../../../DSP/library/includes
INCLUDES += -I../../../Modular/library/includes
INCLUDES += -I../../../Modular/library/PeerObject
INCLUDES += -I../../../Modular/library/ProtocolLib
INCLUDES += -I../../../Modular/library/SchedulerLib
INCLUDES += -I../../../Score/library/includes
INCLUDES += -I/usr/include/libxml2

#########################################

LIBS = ../../../Foundation/library/build/JamomaFoundation.dylib
LIBS += ../../../DSP/library/build/JamomaDSP.dylib
LIBS += ../../../Modular/library/build/JamomaModular.dylib
LIBS += ../../../Score/library/build/JamomaScore.dylib
LIBS += /usr/lib/libxml2.dylib

#########################################

Debug: createdirs make install

Release: createdirs make install

createdirs:
	mkdir -p build

clean:
	rm -f $(SRC32) $(SRC64)
	rm -rf build

make:
	rm -f build/$(NAME) ;
	$(CC_32) main.cpp -g -std=c++11 -stdlib=libc++ -DTT_PLATFORM_MAC -DTT_ENABLE_ASSERTIONS ${INCLUDES} ${LIBS} -o build/$(NAME) ;
	rm -f build/$(NAME) ;
	$(CC_64) main.cpp -g -std=c++11 -stdlib=libc++ -DTT_PLATFORM_MAC -DTT_ENABLE_ASSERTIONS ${INCLUDES} ${LIBS} -o build/$(NAME) ;

install:
	sudo ../../../Shared/jamoma_copy.sh build/$(NAME) /usr/local/bin

