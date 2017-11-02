#Compile Option 

#TOPDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)

TOPDIR = /home/sul/voa0
INSTALL_DIR=
COMPILE_PREFIX = arm-linux-gnueabihf-

#GLOBAL_CFLAGS = -g -O2 --cpu=4 --c99 
#GLOBAL_CFLAGS = -O3 --cpu=4 --c99

# --fpu=SoftVFP 
# --fpmode=ieee_full
# --fpmode=ieee_fixed
#LFLAGS_ROM = --debug --datacompressor off --scatter=D:\works\works_armcc\voyager\scatter.txt --entry _Startup

CC = $(COMPILE_PREFIX)gcc
CXX = $(COMPILE_PREFIX)cpp
AR = $(COMPILE_PREFIX)ar #rc 
ASM   = $(COMPILE_PREFIX)asm

INCLUDEDIRS =

TARGET = remotevoy
DIRS =  
LIBNAME = 

SLIBNAME = $(LIBNAME).a
DLIBNAME = $(LIBNAME).so 

LIBDIR = $(TOPDIR)\libs 
LIBS = --userlibpath=$(LIBDIR) 
BIN_CROSS= 

#--------------------------------------------------------------------------------#
#SRCS = $(wildcard *.c)   select all .c 
#OBJS = $(SRCS:.c=.o)     

OBJS = $(patsubst %.s, %.o, $(wildcard *.s)) $(patsubst %.c, %.o, $(wildcard *.c)) 

INCLUDEDIRS += -I$(TOPDIR)\include  

CFLAGS        = $(INCLUDEDIRS) $(GLOBAL_CFLAGS) -g 
CXXFLAGS      = $(CFLAGS)
LDFLAGS       =  -lpthread

# Compilation target for S files
%.o:%.s
	@echo "Compiling $< ..."
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

# 
# Compilation target for C files 
# 
%.o:%.c 
	@echo "Compiling $< ..." 
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $< 

# 
# Compilation target for C++ files 
# 
%.o:%.cc 
	@echo "C++ compiling $< ..." 
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) -o $@ $< 

all : sub $(TARGET) 
	mkdir -p obj 
	mv *.o obj

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "complete!!  $< ..." 
	
sub :
	@for dir in $(DIRS); do \
	make -C $$dir || exit $?; \
	done

dep : 
	$(CC) -M $(INCLUDEDIRS) $(SRCS) > .depend 

clean : 
	rm -rf $(OBJS) $(TARGET) obj lib
	@for dir in $(DIRS); do \
	make -C $$dir clean $?; \
	done


ifeq (.depend,$(wildcard .depend)) 
include .depend 
endif 
