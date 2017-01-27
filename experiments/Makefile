CXX = g++
CXX_FLAGS = -std=c++11
RM = rm -f

all: bitset

bitset: bitset.o
	$(CXX) $(CXX_FLAGS) -o $@ $?

%.o : %.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $?

clean:
	$(RM) *.o bitset

