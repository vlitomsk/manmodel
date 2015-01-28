CV=`pkg-config --libs --cflags opencv`

all: edge proj

edge:
	rm -f edgeCollector
	g++ -std=c++11 $(CV) edgeCollector.cpp -o edgeCollector

proj:
	rm -f model
	g++ -g -std=c++11 $(CV) model.cpp -o model
