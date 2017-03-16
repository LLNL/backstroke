##############################################################################
# This Makefile installs backstroke on a system where ROSE is installed and
# uses installed tools to grab the environment from autoconf generated info.
# The environment variable ROSE_INSTALLED must be set.

##############################################################################
# Get autoconf info from ROSE installation (see rose-config --help) 
ROSE_INSTALLED = $(shell rose-config prefix)
CXX      = $(shell $(ROSE_INSTALLED)/bin/rose-config cxx)
CPPFLAGS = $(shell $(ROSE_INSTALLED)/bin/rose-config cppflags) -I.
CXXFLAGS = $(shell $(ROSE_INSTALLED)/bin/rose-config cxxflags)
LDFLAGS  = $(shell $(ROSE_INSTALLED)/bin/rose-config ldflags) -L.
LIBDIRS  = $(shell $(ROSE_INSTALLED)/bin/rose-config libdirs)

MOSTLYCLEANFILES =

tool = backstroke

OBJECTS=$(tool).o ClassInfo.o CodeGenerator.o CommandLineOptions.o EditSequence.o PragmaMap.o pragmaparser.o TimeMeasurement.o TransformationSequence.o Utility.o

CPPFLAGS +=-std=c++11
RTSS_LIBRARY_NAME=rtss
RTSS_LIBRARY=rtss/$(RTSS_LIBRARY_NAME)
RTSS_LIBRARY_H=rtss/backstroke/$(RTSS_LIBRARY_NAME).h
RTSS_LIBRARY_MACROS=rtss/backstroke/$(RTSS_LIBRARY_NAME)-macros.h
RTSS_LIBRARY_C=rtss/$(RTSS_LIBRARY_NAME).C

all: $(tool) rtss/librtss.so

##############################################################################
# BACKSTROKE object files

%.o : %.C
	$(CXX) -c $(CPPFLAGS) $< -o $@

$(RTSS_LIBRARY).o: $(RTSS_LIBRARY_H) $(RTSS_LIBRARY_MACROS) $(RTSS_LIBRARY).C
	$(CXX) -c $(CPPFLAGS) -fPIC -Irtss $(RTSS_LIBRARY_C) -o$(RTSS_LIBRARY).o

rtss/librtss.so: $(RTSS_LIBRARY).o
	$(CXX) $(RTSS_LIBRARY).o -shared -ortss/librtss.so

##############################################################################
# BACKSTROKE

$(tool): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

MOSTLYCLEANFILES += $(tool) $(addsuffix .o, $(tool)) $(OBJECTS) $(RTSS_LIBRARY).o $(RTSS_LIBRARY).so


# explicit rules because of multiple bison parsers
# these files are tracked files. They will only be rebuilt if manually deleted.
srcdir=.
pragmaparser.C: $(srcdir)/pragmalexer.ll $(srcdir)/pragmaparser.yy
	@$(YACC) $(YFLAGS) -d -p pragmaparser $(srcdir)/pragmaparser.yy # generates y.tab.h/c
	@$(LEX) $(LFLAGS) $(AM_LFLAGS) -Ppragmaparser $(srcdir)/pragmalexer.ll
	@mv y.tab.h $(srcdir)/pragmaparser.h
	@cat lex.yy.c y.tab.c > $(srcdir)/pragmaparser.C
	@rm lex.yy.c y.tab.c
pragmaparser.h: pragmaparser.C

##############################################################################
# Standard boilerplate

BACKSTROKE_BIN=$(ROSE_INSTALLED)/bin
BACKSTROKE_INCLUDE_PATH=$(ROSE_INSTALLED)/include/backstroke
BACKSTROKE_LIB=$(ROSE_INSTALLED)/lib

.PHONY: clean
clean:
	rm -f $(MOSTLYCLEANFILES)

install: $(tool) $(RTSS_LIBRARY_H) $(RTSS_LIBRARY_MACROS) rtss/lib$(RTSS_LIBRARY_NAME).so
	cp $(tool) $(BACKSTROKE_BIN)
	mkdir -p $(BACKSTROKE_INCLUDE_PATH)
	cp $(RTSS_LIBRARY_H) $(BACKSTROKE_INCLUDE_PATH)
	cp $(RTSS_LIBRARY_MACROS) $(BACKSTROKE_INCLUDE_PATH)
	mkdir -p $(BACKSTROKE_LIB)
	cp rtss/lib$(RTSS_LIBRARY_NAME).so $(BACKSTROKE_LIB)

uninstall:
	rm -f $(ROSE_INSTALLED)/bin/$(tool)
	rm -f $(ROSE_INSTALLED)/lib/lib$(RTSS_LIBRARY_NAME).so
	rm -R -f $(BACKSTROKE_INCLUDE_PATH)
