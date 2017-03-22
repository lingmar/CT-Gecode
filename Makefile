override MKDIR = @mkdir -p

export lib_path = ${GECODE_PATH}/lib
export include_path = ${GECODE_PATH}/include

export CPLUS_INCLUDE_PATH=${include_path}:$CPLUS_INCLUDE_PATH
# export LIBRARY_PATH=${lib_path}:$LIBRARY_PATH 

CXX = g++

OS := $(shell uname)

ifeq ($(OS),Darwin)
LIBS = -L${lib_path} -lgecodeflatzinc -lgecodedriver -lgecodesearch -lgecodeminimodel -lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport
else
LIBS = -L${lib_path} -lgecodeflatzinc -lgecodedriver -lgecodesearch -lgecodeminimodel -lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport -lgecodegist -lpthread
endif

CXX_FLAGS = -std=c++11 -I${include_path} #-I/usr/include/qt4 #-DDEBUG
RM = rm -f

OBJECTS = out/compact-table.o

.PHONY: all clean report test

all: bin/kakuro bin/black-hole-patience bin/swedish-drinking-protocol

out/%.o: src/%.cpp
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -c -o $@ $?	#$(LIBS)

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
	cd gecode/test && make test && ./test -test Extensional::TupleSet
