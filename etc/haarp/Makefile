CXX = g++
CFLAGS = -g -Wall -O2 -lmysqlclient -ldl -Wunused-result -rdynamic

OBJECTS_BASE  = utils.o params.o 
OBJECT_HC1 = haarpClean.o 
OBJECT_HC2 = haarpClean2.o 

all: haarputils

haarputils: $(OBJECTS_BASE) $(OBJECT_HC1) $(OBJECT_HC2)
	$(CXX) -o haarpclean $(OBJECTS_BASE) $(OBJECT_HC1) $(CFLAGS)
	$(CXX) -o haarpclean2 $(OBJECTS_BASE) $(OBJECT_HC2) $(CFLAGS)

utils.o: ../../haarp/utils.cpp ../../haarp/utils.h ../../haarp/default.h
	$(CXX) $(CFLAGS) -c -o utils.o $<

params.o: ../../haarp/params.cpp ../../haarp/params.h ../../haarp/default.h
	$(CXX) $(CFLAGS) -c -o params.o $<

haarpClean.o: haarpClean.cpp haarpClean.h
	$(CXX) $(CFLAGS) -c -o haarpClean.o $<

haarpClean2.o: haarpClean2.cpp haarpClean.h
	$(CXX) $(CFLAGS) -c -o haarpClean2.o $<

clean: 
	rm -f haarpclean haarpclean2 *.o

