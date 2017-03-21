override MKDIR = @mkdir -p

CXX = g++
LIBS = -L/usr/local/lib -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -lgecodedriver -lgecodeminimodel -lgecodeset #-lgecodegist
CXX_FLAGS = -std=c++11 -I/usr/local/include -I/usr/include/qt4 #-DDEBUG
RM = rm -f

OBJECTS = out/compact-table.o

.PHONY: all clean report test

all: bin/kakuro bin/black-hole-patience bin/swedish-drinking-protocol

out/%.o: src/%.cpp
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -c -o $@ $?	

bin/kakuro: $(OBJECTS) out/kakuro.o
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)

bin/black-hole-patience: $(OBJECTS) out/black-hole-patience.o
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)

bin/swedish-drinking-protocol: $(OBJECTS) out/swedish-drinking-protocol.o
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)

report:
	cd report && $(MAKE)

clean:
	$(RM) -r out/ bin/

test:
	cd gecode/test && make test && ./test -test Extensional::TupleSet
