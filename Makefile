.PHONY:all
all:ChatServer
ChatServer::Main.cc
	g++ -o  $@ $^ -std=c++11 -lmuduo_net -lmuduo_base -lpthread -lmysqlclient


.PHONY:clean

clean:
	rm -rf ChatServer
