cc=g++

.PHONY:all
all:client server

client:client.cc
	$(cc) -o $@ $^ -std=c++11
server:server.cc
	$(cc) -o $@ $^ -lpthread -std=c++11

.PHONY:clean
clean:
	rm -f client server
