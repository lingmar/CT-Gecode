CXX = g++
LIBS = -L/usr/local/lib -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -lgecodedriver -lgecodeminimodel -lgecodeset #-lgecodegist
CXX_FLAGS = -I/usr/local/include -I/usr/include/qt4 -std=c++11 #-DDEBUG
RM = rm -f

.PHONY: all swedish-drinking-protocol kakuro black-hole-patience

all: swedish-drinking-protocol kakuro black-hole-patience

swedish-drinking-protocol: compact-table.o swedish-drinking-protocol.o
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS) 

kakuro: compact-table.o kakuro.o
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)

black-hole-patience: compact-table.o black-hole-patience.o
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)	

%.o: %.cpp
	$(CXX) $< $(CXX_FLAGS) -o $@

clean:
	$(RM) *.o bitset swedish-drinking-protocol kakuro black-hole-patience

