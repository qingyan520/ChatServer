.PHONY:all

all:client
client:Main.cc
	g++ -o $@ $^ -std=c++11 -lpthread

.PHONY:clean

clean:
	rm -rf client
