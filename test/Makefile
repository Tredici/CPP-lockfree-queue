CC:=g++
CPPFLAGS:=-Wall -Wextra -g -std=c++17 -pthread
EXE:=
RUN:=

all: run_all

EXE+=test1
test1: test1.cc ../fixed_size_lockfree_queue.hh

RUN+=r_test1
r_test1: test1
	./test1

run_all: $(RUN)

clean:
	rm -f *.o *.d $(EXE)
