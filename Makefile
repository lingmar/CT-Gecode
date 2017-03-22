override MKDIR = @mkdir -p

#LIBRARY_PATH=/it/slask/student/liin3244/gecode/lib
#LD_LIBRARY_PATH=/it/slask/student/liin3244/gecode/lib

export gecode_path = /it/slask/student/liin3244/CT-Gecode/gecode
export lib_path = ${gecode_path}/lib
export include_path = ${gecode_path}/include

export CPLUS_INCLUDE_PATH=${include_path}:$CPLUS_INCLUDE_PATH
# export LIBRARY_PATH=${lib_path}:$LIBRARY_PATH 

CXX = g++
LIBS = -L${lib_path} -lgecodeflatzinc -lgecodedriver -lgecodegist -lgecodesearch -lgecodeminimodel -lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport #-lpthread
CXX_FLAGS = -std=c++11 -I${include_path} #-I/usr/include/qt4 #-DDEBUG
RM = rm -f

OBJECTS = out/compact-table.o

.PHONY: all clean report test

all: bin/kakuro bin/black-hole-patience bin/swedish-drinking-protocol

out/%.o: src/%.cpp
	$(MKDIR) $(@D)
	$(CXX) $(CXX_FLAGS) -c -o $@ $?	$(LIBS)

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
