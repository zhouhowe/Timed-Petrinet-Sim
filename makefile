
FLAGS = $(CPPFLAGS) -O3 #-Wall -Werror
CCFLAGS = $(CPPFLAGS) -O3 #-D_DEBUG_ #-Wall -Werror


OBJ = event.o lcgrand.o petri-net.o

all:	main

main:	$(OBJ)
	g++ -o petri-net $(OBJ) $(FLAGS)

.cpp.o:
	g++ -c $*.cpp $(CCFLAGS)

clean:
	rm -rf *.o petri-net
