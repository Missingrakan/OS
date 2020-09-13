cc=g++

.PHONY:all
all:Client Server

Client:Client.cc
	$(cc) -o $@ $^ -std=c++11
Server:Server.cc
	$(cc) -o $@ $^ -lpthread -std=c++11

.PHONY:clean
clean:
	rm -f Client Server
