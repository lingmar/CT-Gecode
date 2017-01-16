/*
 * Authors: Linnea Ingmar and Mark Tibblin
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <math.h>
#include <assert.h>
#include <cstdio>

using namespace std;
using namespace Gecode;
using namespace Gecode::Driver;

class SwedishDrinkingProtocol : public Script {
public:
  /// Decision variables.
  BoolVarArray x;

  /// The actual problem.
  SwedishDrinkingProtocol(Options& opt)
    : x(*this, 3, 0, 1)
    , Script(opt){

    TupleSet t;
    t.add(IntArgs(3, 0, 0, 0));
    t.add(IntArgs(3, 0, 1, 0));
    t.add(IntArgs(3, 1, 0, 0));
    t.finalize();

    extensional(*this, x, t);

    rel(*this, x[0] == 1);
    
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
    cout << x << endl;
  }
  

};

int main(int argc, char* argv[]) {
  Options opt("SwedishDrinkingProtocol");
  SwedishDrinkingProtocol* sdp = new SwedishDrinkingProtocol(opt);
  DFS<SwedishDrinkingProtocol> e(sdp);
  
  while (SwedishDrinkingProtocol* s = e.next()) {
    sdp->print(cout);
  }
  delete sdp;  
  
}
