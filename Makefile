PROG_RUN 	= multirun

CXX         = g++
CXXFLAGS    = -Wall -O2 -std=c++0x
LINKFLAGS   = -Wall -O2 -pthread

RUN_SRC     = multirun.cpp 
RUN_OBJ     = multirun.o   

.SUFFIXES:
.SUFFIXES: .o .c .cpp
.PHONY: all clean cleanall

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $*.cpp

all: $(PROG_RUN) $(PROGRAM1)

love:
	@echo "You can not make love with me, please find a human partner!"

$(PROG_RUN): $(RUN_OBJ)
	$(CXX) $(LINKFLAGS) -o $(PROG_RUN) $(RUN_OBJ) 

clean:
	-rm -f *.o

cleanall: clean
	-rm $(PROG_RUN)

