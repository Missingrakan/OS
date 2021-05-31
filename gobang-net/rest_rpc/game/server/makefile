BIN=server
INCLUDE_PATH=-I ../../include/ -I ../../thirdparty/msgpack-c/include/ -lpthread

$(BIN):main.cpp
	g++ $^ -o $@ $(INCLUDE_PATH) -g 

.PHONY:clean
clean:
	rm -f $(BIN)
