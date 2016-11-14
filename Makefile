CPP=g++
CPPFLAGS= -O2 -Wall -Wextra -pedantic -std=gnu++14 -g -ggdb
LDFLAGS= -L/home/whj/internship/lmdb-mdb.master/lmdb-mdb.master/libraries/liblmdb -llmdb -lboost_system -lboost_thread
INC=-I /home/whj/internship/lmdb-mdb.master/lmdb-mdb.master/libraries/liblmdb
main : main.o
	$(CPP) -o main main.o $(LDFLAGS)
main.o : main.cpp lmdb++.h postPartition.h
	$(CPP) -c main.cpp $(INC) $(CPPFLAGS)
clean :
	rm main.o
