CXX = g++
CFLAGS = -g -Wall -O2 -lmysqlclient -ldl -Wunused-result -rdynamic

OBJECTS = utils.o params.o haarpClean.o 

all: haarputils

haarputils: $(OBJECTS)
	$(CXX) -o haarpclean $(OBJECTS)  $(CFLAGS)

utils.o: ../../haarp/utils.cpp ../../haarp/utils.h ../../haarp/default.h
	$(CXX) $(CFLAGS) -c -o utils.o $<

params.o: ../../haarp/params.cpp ../../haarp/params.h ../../haarp/default.h
	$(CXX) $(CFLAGS) -c -o params.o $<

haarpClean.o: haarpClean.cpp haarpClean.h
	$(CXX) $(CFLAGS) -c -o haarpClean.o $<

clean: 
	rm -f haarpclean *.o

