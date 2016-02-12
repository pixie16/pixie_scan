#!/bin/make
# GNUmakefile using implicit rules and standard definitions
SHELL=/bin/sh

PIXIE_SUITE_DIR = $(HOME)/Research/PixieSuite

# Uncomment the following line for root functionality
# USEROOT = 1
# Uncomment this line for a more verbose scan
# CXXFLAGS += -DVERBOSE
# Undefine to make a "online" version
# ONLINE = 1
# Define to see debugging information for TreeCorrelator
#DEBUG = 1
#Uncomment this line to use the Pulse Fitting routine
PULSEFIT = 1
# Define to use Gamma-Gamma gates in GeProcessor
# GGATES = 1

#------- instruct make to search through these
#------- directories to find files
vpath %.hpp include/:include/experiment
vpath %.h include/
vpath %.icc include/
vpath %.cpp src/analyzers:src/core:src/experiment:src/processors
vpath %.o obj/

SCAN_LIB_DIR = $(PIXIE_SUITE_DIR)/exec/lib
SCAN_INC_DIR = $(PIXIE_SUITE_DIR)/exec/include

OutPutOpt     = -o # keep whitespace after "-o"

#------- define file suffixes
fSrcSuf   = f
cSrcSuf   = c
c++SrcSuf = cpp
cxxSrcSuf = cxx
ObjSuf    = o


#------- define compilers

GCC       = gcc
CXX       = g++
LINK.o    = $(CXX) $(LDFLAGS)

#------- include directories for the pixie c files
CINCLUDEDIRS += -Iinclude -I$(SCAN_INC_DIR)

# -Dnewreadout is needed to account for a change to pixie16 readout
# structure change on 03/20/08.  Remove for backwards compatability
#
# for debug and profiling add options -g -pg
# and remove -O
#------- define basic compiler flags, no warnings on code that is not our own
FFLAGS   += -O3
GCCFLAGS += -fPIC $(CINCLUDEDIRS)
CXXFLAGS += -Wall -g -fPIC $(CINCLUDEDIRS)

ifdef ONLINE
CXXFLAGS += -DONLINE
endif

#------- basic linking instructions
LDLIBS   += -lm -lstdc++
ifdef PULSEFIT
LDLIBS   += -lgsl -lgslcblas
endif

GCCFLAGS += -O3
CXXFLAGS += -O3

#---------- Include the list of objects
include Makefile.objs

#---------- Change the executable name if necessary
ifdef ONLINE
PIXIE = pixie_ldf_c_online$(ExeSuf)
else
PIXIE = pixie_ldf_c$(ExeSuf)
endif

#---------- Adjust compilation if ROOT capability is desired
ifdef USEROOT
CXX_OBJS  += $(ROOTPROCESSORO)
PIXIE = pixie_ldf_c_root$(ExeSuf)
ROOTCONFIG   := root-config

#no uncomment ROOTCLFAGS   := $(filter-out pthread,$(ROOTCFLAGS))
CXXFLAGS     += $(shell $(ROOTCONFIG) --cflags) -Duseroot
LDFLAGS      += $(shell $(ROOTCONFIG) --ldflags)
LDLIBS       += $(shell $(ROOTCONFIG) --libs)
endif

#------------ Compile with Gamma-Gamma gates support in GeProcessor
ifdef GGATES
CXXFLAGS	+= -DGGATES
endif

#------------ Compile with debug information for TreeCorrelator
ifdef GGATES
CXXFLAGS	+= -DDEBUG
endif

#---------- Update some information about the object files
OBJDIR = ./obj
CXX_OBJDIR = $(OBJDIR)/c++
CXX_OBJS_W_DIR = $(addprefix $(CXX_OBJDIR)/,$(CXX_OBJS))

#--------- Add to list of known file suffixes
.SUFFIXES: .$(cxxSrcSuf) .$(fSrcSuf) .$(c++SrcSuf) .$(cSrcSuf)

all: $(CXX_OBJS_W_DIR) $(PIXIE)

$(CXX_OBJS_W_DIR): | $(CXX_OBJDIR)

$(CXX_OBJDIR):
	mkdir -p $(CXX_OBJDIR)

$(CXX_OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

#----------- link all created objects together
#----------- to create pixie_ldf_c program
$(PIXIE): $(CXX_OBJS_W_DIR) $(LIBS)
	$(LINK.o) $^ -o $@ $(LDLIBS) -L$(SCAN_LIB_DIR) -lPixieScan

.PHONY: clean tidy doc
clean:
	@echo "Cleaning up..."
	@rm -rf ./$(OBJDIR) $(PIXIE) ./core ./*~ ./*.save \
	./src/*/*~ ./include/*~
tidy:
	@echo "Tidying up..."
	@rm -f ./core ./*~ ./src/*/*~ ./include/*~ ./*.save
doc: doc/Doxyfile
	@doxygen $^
