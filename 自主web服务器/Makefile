.PHONY:all
all:http_server testCGI

http_server:main.cc
	g++ -o $@ $^ -std=c++11 -lpthread

testCGI:testCGI.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f http_server testCGI

.PHONY:output
output:
	mv testCGI ./wwwroot/cgi
