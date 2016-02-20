 # makefile to build library and testdriver
.SUFFIXES:.cc
.cc.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

# assign your compiler name to CC if not g++
CC = g++
# where your header files are (. is same directory as the makefile)
INCLUDES=-I.
# turn on debugging symbols
CFLAGS=-g -std=c++11 -std=gnu++11 -std=gnu++0x
#CFLAGS=-g -std=gnu++0x
# where your object files are (. is the same directory as the makefile)
LDFLAGS=-L.

# when "make" is typed at command line with no args "default" will get executed and do "all"
default: 	all

#
# assign source and headers for our "utility" lib to variables below
#
# replace "SBB" (my initials) with yours or use the same filenames
LIB_UTIL_SRC = SBB_date.cc SBB_io.cc calculation.cc SBB_util.cc SBB_ratings.cc main.cc
LIB_UTIL_SRC_HDR = SBB_date.h SBB_io.h calculation.h SBB_util.h SBB_ratings.h
#
LIB_UTIL_OBJ = $(LIB_UTIL_SRC:.cc=.o)
$(LIB_UTIL_OBJ) : $(LIB_UTIL_SRC_HDR)
LIB_UTIL_TARGET = libutil.a

# 
# the file where main() is
MAIN_SRC = sbb_server.c
MAINOBJ = $(MAIN_SRC:.cc=.o) 
$(MAINOBJ) : $(LIB_UTIL_TARGET)
MAINTARGET=testdriver

ARCHIVE_OPTIONS = rucsv

$(LIB_UTIL_TARGET) : $(LIB_UTIL_OBJ)
	ar $(ARCHIVE_OPTIONS) $(LIB_UTIL_TARGET) $(LIB_UTIL_OBJ)

$(MAINTARGET) : $(MAINOBJ) $(LIB_UTIL_TARGET)
	$(CC) $(LDFLAGS) $(MAINOBJ) $(LIB_UTIL_TARGET) -o $(MAINTARGET)

all : util test

test: $(MAINTARGET)

util: $(LIB_UTIL_TARGET)


clean:
	rm -f *.o 

clobber:
	-rm -f *.o *.a $(MAINTARGET)

backup: clobber
	tar cvf ./nyu.tar *.*