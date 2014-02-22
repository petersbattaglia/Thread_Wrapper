all: newthread

newthread: NewThread.cpp Main.cpp NewThread.hpp
	rm -f *.o newthread
	g++ NewThread.cpp Main.cpp -o newthread -std=c++11 -g -lpthread

clean:
	rm -f *.o newthread

