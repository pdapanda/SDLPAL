#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/rixplay.o \
	${OBJECTDIR}/adplug/surroundopl.o \
	${OBJECTDIR}/libmad/synth.o \
	${OBJECTDIR}/libmad/timer.o \
	${OBJECTDIR}/adplug/player.o \
	${OBJECTDIR}/libmad/version.o \
	${OBJECTDIR}/libmad/music_mad.o \
	${OBJECTDIR}/palcommon.o \
	${OBJECTDIR}/script.o \
	${OBJECTDIR}/libmad/layer3.o \
	${OBJECTDIR}/util.o \
	${OBJECTDIR}/getopt.o \
	${OBJECTDIR}/libmad/frame.o \
	${OBJECTDIR}/libmad/decoder.o \
	${OBJECTDIR}/input.o \
	${OBJECTDIR}/game.o \
	${OBJECTDIR}/magicmenu.o \
	${OBJECTDIR}/ending.o \
	${OBJECTDIR}/rngplay.o \
	${OBJECTDIR}/ui.o \
	${OBJECTDIR}/libmad/fixed.o \
	${OBJECTDIR}/adplug/fprovide.o \
	${OBJECTDIR}/global.o \
	${OBJECTDIR}/sound.o \
	${OBJECTDIR}/res.o \
	${OBJECTDIR}/fight.o \
	${OBJECTDIR}/yj1.o \
	${OBJECTDIR}/libmad/bit.o \
	${OBJECTDIR}/adplug/rix.o \
	${OBJECTDIR}/text.o \
	${OBJECTDIR}/font.o \
	${OBJECTDIR}/itemmenu.o \
	${OBJECTDIR}/libmad/stream.o \
	${OBJECTDIR}/adplug/binio.o \
	${OBJECTDIR}/scene.o \
	${OBJECTDIR}/libmad/layer12.o \
	${OBJECTDIR}/adplug/emuopl.o \
	${OBJECTDIR}/play.o \
	${OBJECTDIR}/uibattle.o \
	${OBJECTDIR}/map.o \
	${OBJECTDIR}/adplug/fmopl.o \
	${OBJECTDIR}/libmad/huffman.o \
	${OBJECTDIR}/uigame.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/adplug/binfile.o \
	${OBJECTDIR}/video.o \
	${OBJECTDIR}/palette.o \
	${OBJECTDIR}/battle.o

# C Compiler Flags
CFLAGS=`sdl-config --cflags` 

# CC Compiler Flags
CCFLAGS=`sdl-config --cflags` 
CXXFLAGS=`sdl-config --cflags` 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/sdlpal

dist/Debug/GNU-Linux-x86/sdlpal: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${LINK.cc} `sdl-config --libs` -Xlinker -Map=dist/Debug/GNU-Linux-x86/sdlpal.map -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/sdlpal  ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/rixplay.o: nbproject/Makefile-${CND_CONF}.mk rixplay.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/rixplay.o rixplay.cpp

${OBJECTDIR}/adplug/surroundopl.o: nbproject/Makefile-${CND_CONF}.mk adplug/surroundopl.cpp 
	${MKDIR} -p ${OBJECTDIR}/adplug
	${RM} $@.d
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/adplug/surroundopl.o adplug/surroundopl.cpp

${OBJECTDIR}/libmad/synth.o: nbproject/Makefile-${CND_CONF}.mk libmad/synth.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/synth.o libmad/synth.c

${OBJECTDIR}/libmad/timer.o: nbproject/Makefile-${CND_CONF}.mk libmad/timer.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/timer.o libmad/timer.c

${OBJECTDIR}/adplug/player.o: nbproject/Makefile-${CND_CONF}.mk adplug/player.cpp 
	${MKDIR} -p ${OBJECTDIR}/adplug
	${RM} $@.d
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/adplug/player.o adplug/player.cpp

${OBJECTDIR}/libmad/version.o: nbproject/Makefile-${CND_CONF}.mk libmad/version.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/version.o libmad/version.c

${OBJECTDIR}/libmad/music_mad.o: nbproject/Makefile-${CND_CONF}.mk libmad/music_mad.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/music_mad.o libmad/music_mad.c

${OBJECTDIR}/palcommon.o: nbproject/Makefile-${CND_CONF}.mk palcommon.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/palcommon.o palcommon.c

${OBJECTDIR}/script.o: nbproject/Makefile-${CND_CONF}.mk script.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/script.o script.c

${OBJECTDIR}/libmad/layer3.o: nbproject/Makefile-${CND_CONF}.mk libmad/layer3.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/layer3.o libmad/layer3.c

${OBJECTDIR}/util.o: nbproject/Makefile-${CND_CONF}.mk util.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/util.o util.c

${OBJECTDIR}/getopt.o: nbproject/Makefile-${CND_CONF}.mk getopt.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/getopt.o getopt.c

${OBJECTDIR}/libmad/frame.o: nbproject/Makefile-${CND_CONF}.mk libmad/frame.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/frame.o libmad/frame.c

${OBJECTDIR}/libmad/decoder.o: nbproject/Makefile-${CND_CONF}.mk libmad/decoder.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/decoder.o libmad/decoder.c

${OBJECTDIR}/input.o: nbproject/Makefile-${CND_CONF}.mk input.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/input.o input.c

${OBJECTDIR}/game.o: nbproject/Makefile-${CND_CONF}.mk game.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/game.o game.c

${OBJECTDIR}/magicmenu.o: nbproject/Makefile-${CND_CONF}.mk magicmenu.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/magicmenu.o magicmenu.c

${OBJECTDIR}/ending.o: nbproject/Makefile-${CND_CONF}.mk ending.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/ending.o ending.c

${OBJECTDIR}/rngplay.o: nbproject/Makefile-${CND_CONF}.mk rngplay.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/rngplay.o rngplay.c

${OBJECTDIR}/ui.o: nbproject/Makefile-${CND_CONF}.mk ui.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/ui.o ui.c

${OBJECTDIR}/libmad/fixed.o: nbproject/Makefile-${CND_CONF}.mk libmad/fixed.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/fixed.o libmad/fixed.c

${OBJECTDIR}/adplug/fprovide.o: nbproject/Makefile-${CND_CONF}.mk adplug/fprovide.cpp 
	${MKDIR} -p ${OBJECTDIR}/adplug
	${RM} $@.d
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/adplug/fprovide.o adplug/fprovide.cpp

${OBJECTDIR}/global.o: nbproject/Makefile-${CND_CONF}.mk global.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/global.o global.c

${OBJECTDIR}/sound.o: nbproject/Makefile-${CND_CONF}.mk sound.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/sound.o sound.c

${OBJECTDIR}/res.o: nbproject/Makefile-${CND_CONF}.mk res.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/res.o res.c

${OBJECTDIR}/fight.o: nbproject/Makefile-${CND_CONF}.mk fight.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/fight.o fight.c

${OBJECTDIR}/yj1.o: nbproject/Makefile-${CND_CONF}.mk yj1.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/yj1.o yj1.c

${OBJECTDIR}/libmad/bit.o: nbproject/Makefile-${CND_CONF}.mk libmad/bit.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/bit.o libmad/bit.c

${OBJECTDIR}/adplug/rix.o: nbproject/Makefile-${CND_CONF}.mk adplug/rix.cpp 
	${MKDIR} -p ${OBJECTDIR}/adplug
	${RM} $@.d
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/adplug/rix.o adplug/rix.cpp

${OBJECTDIR}/text.o: nbproject/Makefile-${CND_CONF}.mk text.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/text.o text.c

${OBJECTDIR}/font.o: nbproject/Makefile-${CND_CONF}.mk font.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/font.o font.c

${OBJECTDIR}/itemmenu.o: nbproject/Makefile-${CND_CONF}.mk itemmenu.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/itemmenu.o itemmenu.c

${OBJECTDIR}/libmad/stream.o: nbproject/Makefile-${CND_CONF}.mk libmad/stream.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/stream.o libmad/stream.c

${OBJECTDIR}/adplug/binio.o: nbproject/Makefile-${CND_CONF}.mk adplug/binio.cpp 
	${MKDIR} -p ${OBJECTDIR}/adplug
	${RM} $@.d
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/adplug/binio.o adplug/binio.cpp

${OBJECTDIR}/scene.o: nbproject/Makefile-${CND_CONF}.mk scene.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/scene.o scene.c

${OBJECTDIR}/libmad/layer12.o: nbproject/Makefile-${CND_CONF}.mk libmad/layer12.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/layer12.o libmad/layer12.c

${OBJECTDIR}/adplug/emuopl.o: nbproject/Makefile-${CND_CONF}.mk adplug/emuopl.cpp 
	${MKDIR} -p ${OBJECTDIR}/adplug
	${RM} $@.d
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/adplug/emuopl.o adplug/emuopl.cpp

${OBJECTDIR}/play.o: nbproject/Makefile-${CND_CONF}.mk play.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/play.o play.c

${OBJECTDIR}/uibattle.o: nbproject/Makefile-${CND_CONF}.mk uibattle.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/uibattle.o uibattle.c

${OBJECTDIR}/map.o: nbproject/Makefile-${CND_CONF}.mk map.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/map.o map.c

${OBJECTDIR}/adplug/fmopl.o: nbproject/Makefile-${CND_CONF}.mk adplug/fmopl.c 
	${MKDIR} -p ${OBJECTDIR}/adplug
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/adplug/fmopl.o adplug/fmopl.c

${OBJECTDIR}/libmad/huffman.o: nbproject/Makefile-${CND_CONF}.mk libmad/huffman.c 
	${MKDIR} -p ${OBJECTDIR}/libmad
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/libmad/huffman.o libmad/huffman.c

${OBJECTDIR}/uigame.o: nbproject/Makefile-${CND_CONF}.mk uigame.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/uigame.o uigame.c

${OBJECTDIR}/main.o: nbproject/Makefile-${CND_CONF}.mk main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/adplug/binfile.o: nbproject/Makefile-${CND_CONF}.mk adplug/binfile.cpp 
	${MKDIR} -p ${OBJECTDIR}/adplug
	${RM} $@.d
	$(COMPILE.cc) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/adplug/binfile.o adplug/binfile.cpp

${OBJECTDIR}/video.o: nbproject/Makefile-${CND_CONF}.mk video.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/video.o video.c

${OBJECTDIR}/palette.o: nbproject/Makefile-${CND_CONF}.mk palette.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/palette.o palette.c

${OBJECTDIR}/battle.o: nbproject/Makefile-${CND_CONF}.mk battle.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -D_DEBUG -I/usr/include/SDL -MMD -MP -MF $@.d -o ${OBJECTDIR}/battle.o battle.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/sdlpal

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
