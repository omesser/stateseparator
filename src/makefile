#=============================================================
# Variable Definitions
#=============================================================
#build type
SERVER ?= 1

#compiler
CC := /usr/bin/gcc
CXX := /usr/bin/g++

#directories
OBJDIR := ../build/objs/
LIBDIR := ../build/lib/
BINDIR := ../build/bin/
BACKUPDIR := ../backup/
INCLUDE_DIRS := -I../external/

#optimization flags
#add -flto to the OPT flags when using a modern compiler
OPT := -O3 -fomit-frame-pointer -fgcse-sm -fgcse-las -floop-optimize2 -fmodulo-sched -mfpmath=sse -msse2 -fopenmp
ifeq ($(SERVER),1)
	OPT += -DNDEBUG
endif

#compile flags
ifeq ($(SERVER),1)
	CXXFLAGS := -Wall -Werror -pedantic-errors -D__NRUNONSERVER__ -D__NFULLMT__
else
	CXXFLAGS := -Wall -Werror -pedantic-errors -D__NCOLLECTSTATS__ -D__NSTRICT__ -D__NNORANDOM__ -D__NFULLMT__
endif
OUTFILE = -o $@

#object flags
OBJFLAGS := -c $(CXXFLAGS)

#shared object flags
SOFLAGS := $(CXXFLAGS) -fPIC -shared

#linker flags
LDFLAGS := -Wl,-s -Wl,--as-needed -lm -lgomp -lpthread

#headers
BASIC_HEADERS := basic_def.h NError.h logger.h
MATH_HEADERS := NMathUtils.h $(BASIC_HEADERS)
UTIL_HEADERS := $(MATH_HEADERS) NCollector.h
IO_HEADERS := NInputHandler.h NOutputHandler.h

#targets
UTILS_OBJS := $(OBJDIR)NCollector.o $(OBJDIR)logger.o $(OBJDIR)NError.o $(OBJDIR)NMathUtils.o
TP_OBJS := $(OBJDIR)NThreadPool.o $(OBJDIR)NThreadPool_l.o
IO_OBJS := $(OBJDIR)NInputHandler.o $(OBJDIR)NOutputHandler.o
CORE_OBJS := $(OBJDIR)NPureState.o $(OBJDIR)NMixedState.o
SEPARATOR_OBJS := $(OBJDIR)NRandomGenerator.o $(OBJDIR)NSeparator.o $(OBJDIR)NTester.o $(OBJDIR)NPeresTester.o


#=============================================================
# make Rules
#=============================================================
#targets
all: dir utils threadpool io core separator $(BINDIR)NSeparator $(BINDIR)NRandomizer

utils: $(UTILS_OBJS)

threadpool: $(TP_OBJS)

io: $(IO_OBJS)

core: $(CORE_OBJS)

separator: $(SEPARATOR_OBJS)

#install the tool to the website - available only when building for the server
ifeq ($(SERVER),1)
install: dir $(BINDIR)NSeparator $(BINDIR)NRandomizer
	cp $(BINDIR)NSeparator ../../public_html/NSeparator
	cp $(BINDIR)NRandomizer ../../public_html/NRandomizer
endif

#make a backup archive of the current sources
backup:
	tar czf `date +%F-%H%M`.tar.gz *
	mv *.tar.gz $(BACKUPDIR)

#create the intermediate and target directories
dir:
	mkdir -p $(OBJDIR) $(LIBDIR) $(BINDIR)

#cleanup
clean:
	rm -rf ../build 

#headers
NThreadPool_l.h: NThreadPool.h
	-ln -s NThreadPool.h NThreadPool_l.h

#executables
$(BINDIR)NSeparator: $(OBJDIR)main.o $(UTILS_OBJS) $(TP_OBJS) $(CORE_OBJS) $(SEPARATOR_OBJS) $(IO_OBJS)
	$(CXX) $(OPT) $(OUTFILE) $^ $(LDFLAGS)

$(BINDIR)NRandomizer: $(OBJDIR)NRandomizer.o $(UTILS_OBJS) $(OBJDIR)NPureState.o $(OBJDIR)NRandomGenerator.o $(IO_OBJS)
	$(CXX) $(OPT) $(OUTFILE) $^ $(LDFLAGS)

#special build rules
$(OBJDIR)NPeresTester.o: NPeresTester.cpp NPeresTester.h NOutputHandler.h $(UTIL_HEADERS)
	$(CXX) $(OBJFLAGS) $(OPT) $(INCLUDE_DIRS) $(OUTFILE) $<

$(OBJDIR)NPureState.o: NPureState.cpp NPureState.h NOutputHandler.h $(MATH_HEADERS)
	$(CXX) $(OBJFLAGS) $(OPT) $(INCLUDE_DIRS) $(OUTFILE) $<

$(OBJDIR)NMixedState.o: NMixedState.cpp NMixedState.h NPureState.h NThreadPool.h NOutputHandler.h $(UTIL_HEADERS)
	$(CXX) $(OBJFLAGS) $(OPT) $(INCLUDE_DIRS) $(OUTFILE) $<

$(OBJDIR)NSeparator.o: NSeparator.cpp NSeparator.h NMixedState.h NPureState.h NRandomGenerator.h $(UTIL_HEADERS)
	$(CXX) $(OBJFLAGS) $(OPT) $(INCLUDE_DIRS) $(OUTFILE) $<

$(OBJDIR)NTester.o: NTester.cpp NTester.h NSeparator.h NMixedState.h NRandomGenerator.h $(UTIL_HEADERS)
	$(CXX) $(OBJFLAGS) $(OPT) $(INCLUDE_DIRS) $(OUTFILE) $<

$(OBJDIR)main.o: main.cpp $(IO_HEADERS) NSeparator.h NMixedState.h $(MATH_HEADERS)
	$(CXX) $(OBJFLAGS) $(OPT) $(INCLUDE_DIRS) $(OUTFILE) $<

$(OBJDIR)NRandomizer.o: NRandomizer.cpp $(IO_HEADERS) NRandomGenerator.h NPureState.h $(MATH_HEADERS)
	$(CXX) $(OBJFLAGS) $(OPT) $(INCLUDE_DIRS) $(OUTFILE) $<

#default build rules
$(OBJDIR)%.o: %.cpp %.h $(BASIC_HEADERS)
	$(CXX) $(OBJFLAGS) $(OPT) $(INCLUDE_DIRS) $(OUTFILE) $<

$(LIBDIR)lib%.so: %.o
	$(CXX) $(SOFLAGS) $(OPT) $(OUTFILE) $< $(LDFLAGS)
