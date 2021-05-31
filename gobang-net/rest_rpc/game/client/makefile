BIN=client
INCLUDE_PATH=-I ../../include/ -I ../../thirdparty/msgpack-c/include/ -lpthread

$(BIN):main.cpp
	g++ $^ -o $@ $(INCLUDE_PATH) 

.PHONY:clean
clean:
	rm -f $(BIN)
