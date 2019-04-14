
##############################################################################
# This Makefile installs backstroke on a system where ROSE is installed and
# uses installed tools to grab the environment from autoconf generated info.

##############################################################################
# Get autoconf info from ROSE installation (see rose-config --help) 
ROSE_INSTALLED = $(shell rose-config prefix)
CXX      = $(shell $(ROSE_INSTALLED)/bin/rose-config cxx)
CPPFLAGS = $(shell $(ROSE_INSTALLED)/bin/rose-config cppflags) -I.
CXXFLAGS = $(shell $(ROSE_INSTALLED)/bin/rose-config cxxflags)
LDFLAGS  = $(shell $(ROSE_INSTALLED)/bin/rose-config ldflags) -L.
LIBDIRS  = $(shell $(ROSE_INSTALLED)/bin/rose-config libdirs)

MOSTLYCLEANFILES =

EXTRA_DIST =
bin_PROGRAMS =

##############################################################################
# setup and run tests

srcdir=.
top_builddir=..

BS_INPUT_LANG_STANDARD=c++11
BS_OUTPUT_LANG_STANDARD=c++11

BACKSTROKE_PREPROCESSING="-edg:E -edg:P"
BACKSTROKE_EXE=$(top_builddir)/src/backstroke
BACKSTROKE_BUILD_TEST_DIR=$(top_builddir)/tests
BACKSTROKE_BACKEND=
BACKSTROKE_MODE=
BACKSTROKE_LIB=rtss
BACKSTROKE_LIB_OBJ=rtss.o
BS_LIB_INCLUDE_OPTION=-include backstroke/rtss.h
EXTRA_LIBS=-lpthread
#BACKSTROKE_LIB_OBJ=

#TESTDRIVER_FLAGS=-O3 -fsanitize=address -fno-omit-frame-pointer -g3
TESTDRIVER_FLAGS=-O3 -finline-limit=1000000

NUMOPS=50
NUMEVENTS=50000
ARRAYSIZE=10000

#CPP_FLAGS=-Wall -Werror -Wfatal-errors
CPP_FLAGS=-Wfatal-errors # generated pragmas cause warnings of unused pragma

TESTDRIVER2=testdriverD
FEATURETESTDRIVER=featuretestdriver

BACKSTROKE_LIB_TEST=bs-lib-test

#REVERSIBLE_STDLIB_OPTIONS=-nostdlib -nodefaultlibs -lgcc -I$(REVERSIBLE_STDLIB_HEADERS)
#-nostdinc++

#MY_INCLUDES_REVERSIBLE_STDLIB_HEADERS=-I$(REVERSIBLE_STDLIB_HEADERS)
MY_INCLUDES_REVERSIBLE_STDLIB_HEADERS=
MY_INCLUDES=$(MY_INCLUDES_REVERSIBLE_STDLIB_HEADERS) -I$(srcdir)/../src -I$(srcdir)/../tests -I$(srcdir)/../src/rtss

#REVERSIBLE_STDLIB_DEPENDENT_TESTS_OPTION=-DREVERSIBLE_STDLIB_DEPENDENT_TESTS
REVERSIBLE_STDLIB_DEPENDENT_TESTS_OPTION=

all:

check: check-local

check-local: backstroke.passed # check-stl-containers

backstroke.passed: $(BACKSTROKE_LIB_OBJ)
	@echo "Checking rtss library ..."
	@$(CXX) $(MY_INCLUDES) $(srcdir)/$(BACKSTROKE_LIB_TEST).C $(BACKSTROKE_LIB_OBJ) $(EXTRA_LIBS) -o$(BACKSTROKE_LIB_TEST);
	@./bs-lib-test
	@echo "PASS"

	@echo "Checking backstroke ..."
	@for testno in 1 2 3 4 5; do \
	echo "Feature test $${testno} " ;\
	$(BACKSTROKE_EXE) --no-preprocessor $(BACKSTROKE_BACKEND) $(MY_INCLUDES) $(TESTDRIVER_FLAGS) --edg:no_warnings -I$(srcdir)/../src/rtss $(srcdir)/featuretests/featuretest$${testno}.C || exit 1; \
	$(CXX) $(MY_INCLUDES) $(BS_LIB_INCLUDE_OPTION) -I. $(TESTDRIVER_FLAGS) featuretests/backstroke_featuretest$${testno}.C $(BACKSTROKE_LIB_OBJ) $(EXTRA_LIBS) -obackstroke_featuretest$${testno}; \
	done

	@for testno in 1 2 3 4 5 6 7 8 9 10; do \
	echo "Compile Test $${testno} " ; \
	$(BACKSTROKE_EXE) --no-preprocessor $(BACKSTROKE_MODE) $(BACKSTROKE_BACKEND) -I$(srcdir)/../src/rtss $(srcdir)/test$${testno}.C || exit 1; \
	done

	@echo "---------"
	@echo "Run tests"
	@echo "---------"
	@echo "--- Checking Array Alloc/Dealloc Operations -----------------"
	@$(CXX) $(CPP_FLAGS) $(srcdir)/testdriverA.C -D'ORIGINAL_TESTFILE="test2.C"' -D'REVERSE_TESTFILE="backstroke_test2.C"' $(TESTDRIVER_FLAGS) -I$(srcdir)/../src/ -I$(srcdir)/../src/rtss -o$(BACKSTROKE_BUILD_TEST_DIR)/testdriver2 -I$(BACKSTROKE_BUILD_TEST_DIR) $(BACKSTROKE_LIB_OBJ) $(EXTRA_LIBS)
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver2 $(NUMOPS) 0
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver2 $(NUMOPS) 1
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver2 $(NUMOPS) 2
	@echo

	@$(CXX) $(CPP_FLAGS) $(srcdir)/testdriverA.C -D'ORIGINAL_TESTFILE="test9.C"' -D'REVERSE_TESTFILE="backstroke_test9.C"' $(TESTDRIVER_FLAGS) -I$(srcdir)/../src/ -I$(srcdir)/../src/rtss -o$(BACKSTROKE_BUILD_TEST_DIR)/testdriver9 -I$(BACKSTROKE_BUILD_TEST_DIR) $(BACKSTROKE_LIB_OBJ) $(EXTRA_LIBS)
	@echo "--- Checking Pointer Arithmetic Operations -----------------"
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver9 $(NUMOPS) 0
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver9 $(NUMOPS) 1
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver9 $(NUMOPS) 2

	@$(CXX) $(srcdir)/testdriverA.C -D'ORIGINAL_TESTFILE="test5.C"' -D'REVERSE_TESTFILE="backstroke_test5.C"' $(TESTDRIVER_FLAGS) -I$(srcdir)/../src/ -I$(srcdir)/../src/rtss -o$(BACKSTROKE_BUILD_TEST_DIR)/testdriver5 -I$(BACKSTROKE_BUILD_TEST_DIR) $(BACKSTROKE_LIB_OBJ) $(EXTRA_LIBS)
	@echo "---- Checking Array Memory Allocation (Array) ----------------"
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver5 200000 0
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver5 200000 1
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver5 200000 2

	@$(CXX) $(srcdir)/$(TESTDRIVER2).C -D'ORIGINAL_TESTFILE="test6.C"' -D'REVERSE_TESTFILE="backstroke_test6.C"' $(TESTDRIVER_FLAGS) -I$(srcdir)/../src/ -I$(srcdir)/../src/rtss -o$(BACKSTROKE_BUILD_TEST_DIR)/testdriver6 -I$(BACKSTROKE_BUILD_TEST_DIR) $(BACKSTROKE_LIB_OBJ) $(EXTRA_LIBS)
	@echo
	@echo "Checking Array Operations (random updates) (Array) [alloc mode: default]"
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver6 $(NUMEVENTS) 0 $(NUMOPS) $(ARRAYSIZE)
	@mv measurements.txt array-measurements-mode0-100K.txt
	@echo
	@echo "Checking Array Operations (random updates) (Array) [alloc mode: pre-alloc]"
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver6 $(NUMEVENTS) 1 $(NUMOPS) $(ARRAYSIZE)
	@mv measurements.txt array-measurements-mode1-100K.txt

	@$(CXX) $(srcdir)/$(TESTDRIVER2).C -D'ORIGINAL_TESTFILE="test7.C"' -D'REVERSE_TESTFILE="backstroke_test7.C"' $(TESTDRIVER_FLAGS) -I$(srcdir)/../src/ -I$(srcdir)/../src/rtss -o$(BACKSTROKE_BUILD_TEST_DIR)/testdriver7 -I$(BACKSTROKE_BUILD_TEST_DIR) $(BACKSTROKE_LIB_OBJ) $(EXTRA_LIBS)
	@echo
	@echo "Checking List Operations (random updates) (List) [alloc mode: default]"
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver7 $(NUMEVENTS) 0 $(NUMOPS) 100
	@mv measurements.txt list-measurements-mode0-100K.txt
	@echo
	@echo "Checking List Operations (random updates) (List) [alloc mode: pre-alloc]"
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver7 $(NUMEVENTS) 1 $(NUMOPS) 100
	@mv measurements.txt list-measurements-mode1-100K.txt

	@$(CXX) $(srcdir)/$(TESTDRIVER2).C -D'ORIGINAL_TESTFILE="test10.C"' -D'REVERSE_TESTFILE="backstroke_test10.C"' $(TESTDRIVER_FLAGS) -I$(srcdir)/../src/ -I$(srcdir)/../src/rtss -o$(BACKSTROKE_BUILD_TEST_DIR)/testdriver10 -I$(BACKSTROKE_BUILD_TEST_DIR) $(BACKSTROKE_LIB_OBJ) $(EXTRA_LIBS)
	@echo
	@echo "Checking List<int> Operations (random updates) (List<int>) [alloc mode: default]"
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver10 $(NUMEVENTS) 0 $(NUMOPS) 100
	@echo
	@echo "Checking List<int> Operations (random updates) (List<int>) [alloc mode: pre-alloc]"
	@$(BACKSTROKE_BUILD_TEST_DIR)/testdriver10 $(NUMEVENTS) 1 $(NUMOPS) 100

check-stl-containers: $(BACKSTROKE_LIB_OBJ)
	@echo "STL container run tests"
#	$(CXX) -E -P -std=$(BS_INPUT_LANG_STANDARD) $(MY_INCLUDES) $(srcdir)/teststlcontainers.C > teststlcontainers.pp.C
	$(BACKSTROKE_EXE) --preprocessor $(MY_INCLUDES) $(REVERSIBLE_STDLIB_DEPENDENT_TESTS_OPTION) -std=$(BS_INPUT_LANG_STANDARD) $(BACKSTROKE_MODE) $(BACKSTROKE_BACKEND) --edg:no_warnings -I../src/rtss -I../src --ignore-unions teststlcontainers.C
	$(CXX) -O3 -std=c++11 backstroke_teststlcontainers.C -o$(BACKSTROKE_BUILD_TEST_DIR)/teststlcontainers $(BACKSTROKE_LIB_OBJ) -Wfatal-errors $(EXTRA_LIBS)
	@echo
	@echo "Checking STL containers with headers, Operations (specific updates) [alloc mode: default]"
	@$(BACKSTROKE_BUILD_TEST_DIR)/teststlcontainers 10 0 $(NUMOPS) 100
#	@echo "Checking STL containers with headers, Operations (specific updates) [alloc mode: pre-alloc]"
#	@$(BACKSTROKE_BUILD_TEST_DIR)/teststlcontainers 10 1 $(NUMOPS) 100
	@echo "------------------------------------------------------------------------"

backstroke.fail:
	for testno in 1 2 ; do \
        echo " Test No: $${testno} " ; \
        $(BACKSTROKE_EXE) $(BACKSTROKE_MODE) $(BACKSTROKE_BACKEND) -rose:skipfinalCompileStep -I$(srcdir)/../src/rtss -check $(srcdir)/testFail$${testno}.C ; \
        done

clean: clean-local

clean-local:
	rm -f $(BACKSTROKE_LIB_TEST)
	rm -f *.ti *.dot array-measurements-mode*.txt list-measurements-mode*.txt
	rm -f testdriver? testdriver??
	rm -f measurements.txt
	rm -f testdriverD.pp.C
	rm -f backstroke_featuretest*
	rm -f teststlcontainers teststlcontainers.pp.C backstroke_teststlcontainers.pp.C teststlcontainers.pp.ti
	rm -f test_*.* backstroke_*.* featuretests/backstroke_*.*
	rm -f *.pass.C *.fail.C
	rm -f rtss.o
	rm -f a.out
	rm -f *.pp.C
	rm -f featuretest/backstroke_*.C

${BACKSTROKE_LIB}.o: $(srcdir)/../src/rtss/backstroke/${BACKSTROKE_LIB}.h $(srcdir)/../src/rtss/${BACKSTROKE_LIB}.C
	@$(CXX) -I$(srcdir)/../tests -I$(srcdir)/../src/ -I$(srcdir)/../src/rtss $(srcdir)/../src/rtss/rtss.C -c -o$(BACKSTROKE_LIB).o

check-extended: check check-stl

check-stl: 
	BS_TESTS_PATH=$(srcdir) $(srcdir)/bs-stl-eval.sh
