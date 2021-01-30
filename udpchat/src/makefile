SVR_BIN=ChatServer
CLI_BIN=ChatClient

all:ChatServer ChatClient
$(SVR_BIN):ChatServer.cpp
	g++ -o $@ $^ -g -lpthread -std=c++11

$(CLI_BIN):ChatClient.cpp
	g++ -o $@ $^ -g -lpthread -std=c++11

.PHONY:clean
clean:
	rm -f $(SVR_BIN) $(CLI_BIN)
