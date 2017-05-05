#!/bin/bash

MACRO=$1

cd $GECODE_PATH

touch ./gecode/flatzinc/registry.cpp

g++ -I. -Qunused-arguments -fimplement-inlines -fno-inline-functions -fvisibility=hidden -ggdb -pipe -Wall -Wextra -fPIC -pthread -D $MACRO -c -o gecode/flatzinc/registry.o  gecode/flatzinc/registry.cpp

g++ -dynamiclib -Wl,-single_module -pthread gecode/flatzinc/flatzinc.o gecode/flatzinc/registry.o gecode/flatzinc/parser.tab.o gecode/flatzinc/lexer.yy.o -compatibility_version 43.0 -current_version 43.0 -install_name /Users/linneaingmar/Documents/Kurser/exjobb/gecode/lib/libgecodeflatzinc.43.dylib -L. -lgecodesupport -lgecodekernel -lgecodesearch -lgecodeint -lgecodeset -lgecodefloat -lgecodeminimodel  -lgecodedriver -o libgecodeflatzinc.43.0.dylib

ln -fs libgecodeflatzinc.43.0.dylib libgecodeflatzinc.dylib
ln -fs libgecodeflatzinc.43.0.dylib libgecodeflatzinc.43.dylib

g++ -o tools/flatzinc/fzn-gecode tools/flatzinc/fzn-gecode.o -L. -I. -Qunused-arguments -fimplement-inlines -fno-inline-functions -fvisibility=hidden -ggdb -pipe -Wall -Wextra -fPIC -pthread -lgecodeflatzinc -lgecodedriver  -lgecodesearch -lgecodeminimodel -lgecodeset -lgecodefloat  -lgecodeint -lgecodekernel -lgecodesupport

perl ./misc/fixmanifest.perl .43.0.dylib tools/flatzinc/fzn-gecode.manifest

make install
