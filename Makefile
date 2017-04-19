override MKDIR = @mkdir -p

export lib_path = ${GECODE_PATH}/lib
export include_path = ${GECODE_PATH}/include

export CPLUS_INCLUDE_PATH=${include_path}:$CPLUS_INCLUDE_PATH
# export LIBRARY_PATH=${lib_path}:$LIBRARY_PATH 

CXX = g++

OS := $(shell uname)

# Gist does not work on my MacOS
ifeq ($(OS),Darwin)
LIBS = -L${lib_path} -lgecodeflatzinc -lgecodedriver -lgecodesearch -lgecodeminimodel -lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport
else
LIBS = -L${lib_path} -lgecodeflatzinc -lgecodedriver -lgecodesearch -lgecodeminimodel -lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport -lgecodegist -lpthread
endif

CXX_FLAGS = -I${include_path} -fno-inline-small-functions #-O3 #-DNDEBUG
RM = rm -f

OBJECTS = out/compact-table.o out/compact-table-buggy.o

.PHONY: all clean report test

all: bin/black-hole-patience bin/kakuro #bin/swedish-drinking-protocol

out/%.o: src/%.cpp
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -c -o $@ $?

bin/kakuro: out/kakuro.o
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)

bin/black-hole-patience: out/black-hole-patience.o
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)

bin/swedish-drinking-protocol: out/swedish-drinking-protocol.o
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -o $@ $? $(LIBS)

report:
	cd report && $(MAKE)

clean:
	$(RM) -r out/ bin/

test:
	touch gecode/test/int/extensional.cpp && cd gecode && make test && test/test -test Extensional::TupleSet

flatzinc:
	touch gecode/gecode/flatzinc/registry.cpp && cd gecode && make
