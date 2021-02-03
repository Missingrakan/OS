SVR_BIN=ChatServer
CLI_BIN=ChatClient
INCLUDE_PATH=-I /usr/include/jsoncpp

all:ChatServer ChatClient
$(SVR_BIN):ChatServer.cpp
	g++ -o $@ $^ -g -lpthread -std=c++11 -ljsoncpp $(INCLUDE_PATH)

$(CLI_BIN):ChatClient.cpp
	g++ -o $@ $^ -g -lpthread -std=c++11 -ljsoncpp $(INCLUDE_PATH)

.PHONY:clean
clean:
	rm -f $(SVR_BIN) $(CLI_BIN)
