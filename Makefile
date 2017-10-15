CC       := g++
PERL     := perl

# Flags
OPTFLAGS  := -O0
PROFFLAGS := #-pg
DEFS      += 
CFLAGS    := -Wall -g $(PROFFLAGS) $(OPTFLAGS) $(DEFS)
CPPFLAGS  := $(CFLAGS) -std=c++11 -I/usr/include/boost -I../include
LDFLAGS   := 

# Libraries
LDLIBS    := -lboost_thread \
             -lboost_filesystem \
             -lboost_regex \
             -lboost_system \
             -lboost_date_time \
             -lboost_chrono
LOADLIBES := 


# Target names
UTESTTARGET := unitTest
PTESTTARGET := perfTest
TARGET    :=
LIBTARGET := 

# Directories to build
OBJDIR := obj/
DEPDIR := dep/


# Source files
LIBSRCS   := 
UTESTSRCS := unitTest.cpp
PTESTSRCS := perfTest.cpp
SRCS      := $(UTESTSRCS) $(PTESTSRCS)

# Object files
LIBOBJS   := $(addprefix $(OBJDIR),$(LIBSRCS:.cpp=.o))
UTESTOBJS := $(addprefix $(OBJDIR),$(UTESTSRCS:.cpp=.o))
PTESTOBJS  := $(addprefix $(OBJDIR),$(PTESTSRCS:.cpp=.o))
OBJS      := $(PTESTOBJS) $(UTESTOBJS)


$(UTESTTARGET): $(UTESTOBJS) $(LIBTARGET)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(PROF) -o $@

$(PTESTTARGET): $(PTESTOBJS) $(LIBTARGET)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(PROF) -o $@

$(TARGET): $(OBJS) $(LIBTARGET)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(PROF) -o $@

$(LIBTARGET): $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $^
	ranlib $@

.PHONY: perf
perf:
	$(MAKE) perfTest OPTFLAGS=-O3

.PHONY: clean
clean:
	rm -f $(TARGET) $(UTESTTARGET) $(PTESTTARGET) \
	$(TARGET).exe $(UTESTTARGET).exe $(PTESTTARGET).exe \
	$(LIBTARGET) $(OBJS) $(LIBOBJS) $(DEPDIR)*.d *.bak *.exe.* *~


# Include dependency files
include $(addprefix $(DEPDIR),$(SRCS:.cpp=.d))

$(DEPDIR)%.d : %.c
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< | sed "s@$*.o@$(OBJDIR)& $@@g " > $@'

$(DEPDIR)%.d : %.cpp
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< | sed "s@$*.o@$(OBJDIR)& $@@g " > $@'

$(OBJDIR)%.o: %.c $(DEPDIR)%.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OBJDIR)%.o: %.cc $(DEPDIR)%.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OBJDIR)%.o: %.cpp $(DEPDIR)%.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

%.i : %.c
	$(CC) -E $(CPPFLAGS) $<
