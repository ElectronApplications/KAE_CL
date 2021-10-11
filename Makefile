CXX ?= g++
CXXFLAGS ?= -O3
LIBS = -lcurl -ljsoncpp -lOpenCL

CXXFLAGS := ${CXXFLAGS} -Wall -Wextra -pthread

all:
	${CXX} *.cpp -o miner ${LIBS} ${CXXFLAGS}
debug:
	${CXX} -g *.cpp -o miner ${LIBS} ${CXXFLAGS} -O0
clean:
	rm -f miner
