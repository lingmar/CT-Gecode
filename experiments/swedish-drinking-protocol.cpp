/*
 * Authors: Linnea Ingmar and Mark Tibblin
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <math.h>
#include <assert.h>
#include <cstdio>
#include "../experiments/compact-table.hpp"


using namespace std;
using namespace Gecode;
using namespace Gecode::Driver;

class SwedishDrinkingProtocol : public Script {
public:
  /// Decision variables.
  IntVarArray x;

  /// The actual problem.
  SwedishDrinkingProtocol(Options& opt)
    : x(*this, 2, 1, 3)
    , Script(opt){

    TupleSet t;
    t.add(IntArgs(2, 1, 1));
    t.add(IntArgs(2, 1, 2));
    t.add(IntArgs(2, 1, 3));
    t.finalize();
    
    TupleSet t1;
    t1.add(IntArgs(2, 2, 3));
    t1.finalize();
    
    //distinct(*this, x);

    //rel(*this, x[1] != 1);

    //extensional(*this, x, t);
    switch (opt.model()) {
    case 0: {
      extensional(*this, x, t);
      extensional(*this, x, t1);
      break;
    }
    case 1: {
      extensional2(*this, x, t);
      extensional2(*this, x, t1);
      break;
    }
    default:
      break;
    }

    //rel(*this, x[0] == 1);
    
    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Constructor for cloning \a s
  SwedishDrinkingProtocol(bool share, SwedishDrinkingProtocol& sp) : Script(share, sp) {
    
  }

  /// Perform copying during cloning
  virtual Space* copy(bool share) {
    return new SwedishDrinkingProtocol(share, *this);
  }
  
  /// Print solution in default format (tailored for the python script)
  virtual void print(std::ostream& os) const {
    os << x << endl;
  }
  

};

int main(int argc, char* argv[]) {
  Options opt("SwedishDrinkingProtocol");
  opt.model(0, "g", "extensional");
  opt.model(1, "c", "compact table");
  opt.parse(argc, argv);
  
  SwedishDrinkingProtocol* sdp = new SwedishDrinkingProtocol(opt);
  DFS<SwedishDrinkingProtocol> e(sdp);
  
  while (SwedishDrinkingProtocol* s = e.next()) {
    sdp->print(cout);
  }
  delete sdp;  
  
}
