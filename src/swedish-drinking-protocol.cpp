/*
 * Authors: Linnea Ingmar and Mark Tibblin
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <math.h>
#include <assert.h>
#include <cstdio>
#include "compact-table.cpp"


using namespace std;
using namespace Gecode;
using namespace Gecode::Driver;

class SwedishDrinkingProtocol : public Script {
public:
  /// Decision variables.
  IntVarArray x;

  /// The actual problem.
  SwedishDrinkingProtocol(const Options& opt)
    : x(*this, 3, 1, 3)
    , Script(opt){

    TupleSet t;

    // for (int i = 0; i < 120; i++) {
    //   t.add(IntArgs::create(3,i));
    // }
    t.add(IntArgs(3,1,1,1));
    t.add(IntArgs(3,1,1,1));
    t.add(IntArgs(3,1,1,1));
    t.add(IntArgs(3,1,2,3));
    
    t.finalize();

    IntArgs::create(INT_MAX + 1,0);
    
    TupleSet t0;
    //t0.add(IntArgs(3,1,2,3));
    t0.finalize();

    
    //extensional(*this, x, t);
    switch (opt.model()) {
    case 0: {
      extensional(*this, x, t);
      extensional(*this, x, t0);
      break;
    }
    case 1: {
      extensional2(*this, x, t);
      //extensional2(*this, x, t0);
      break;
    }
    default:
      break;
    }

    //rel(*this, x[3] != 1);
    
    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Constructor for cloning \a s
  SwedishDrinkingProtocol(bool share, SwedishDrinkingProtocol& sp) : Script(share, sp) {
    x.update(*this, share, sp.x);
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

  Script::run<SwedishDrinkingProtocol,DFS,Options>(opt);
  
  // SwedishDrinkingProtocol* sdp = new SwedishDrinkingProtocol(opt);
  // DFS<SwedishDrinkingProtocol> e(sdp);
  // //Script::run<SwedishDrinkingProtocol,DFS,Options>(opt);
  //  while (SwedishDrinkingProtocol* s = e.next()) {
  //    sdp->print(cout);
  //  }
  //delete sdp;  
  
}
