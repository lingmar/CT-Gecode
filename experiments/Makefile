CXX = g++
LIBS = -L/usr/local/lib -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -lgecodedriver -lgecodeminimodel -lgecodeset  -lboost_regex #-lgecodegist
CXX_FLAGS = -I/usr/local/include -I/usr/include/qt4 -std=c++11
RM = rm -f

all: bitset

bitset-test: bitset
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)	

%.o : %.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $?

clean:
	$(RM) *.o bitset-test

