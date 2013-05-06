PROG_RUN    = multirun
PROG_CTRL   = multictrl

CXX         = g++
CXXFLAGS    = -Wall -O2 -std=c++0x
LINKFLAGS   = -Wall -O2 -pthread

RUN_SRC     = multirun.cpp 
RUN_OBJ     = multirun.o   

CTRL_SRC    = multictrl.cpp 
CTRL_OBJ    = multictrl.o   

.SUFFIXES:
.SUFFIXES: .o .c .cpp
.PHONY: all clean cleanall

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $*.cpp

all: $(PROG_RUN) $(PROG_CTRL)

love:
	@echo "You can not make love with me, please find a human partner!"

$(PROG_RUN): $(RUN_OBJ)
	$(CXX) $(LINKFLAGS) -o $(PROG_RUN) $(RUN_OBJ) 

$(PROG_CTRL): $(CTRL_OBJ)
	$(CXX) $(LINKFLAGS) -o $(PROG_CTRL) $(CTRL_OBJ) 

clean:
	-rm -f *.o

cleanall: clean
	-rm $(PROG_RUN)
	-rm $(PROG_CTRL)

