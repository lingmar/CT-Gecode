/*
 *  Main author:
 *     Christian Schulte <cschulte@kth.se>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/int.hh>

using namespace Gecode;
using namespace Gecode::Int;

void extensional2(Home home,
                  const IntVarArgs& x, const IntArgs& w,
                  const IntVarArgs& y, const IntArgs& h);

// The no-overlap propagator
class CompactTable : public Propagator {
protected:  
  // The variables
  ViewArray<IntView> x;
  // The table
  TupleSet t;
    
public:
  // Create propagator and initialize
  CompactTable(Home home,
               ViewArray<IntView>& x0, 
               TupleSet t0)
    : Propagator(home), x(x0), t(t0)
  {
    x.subscribe(home,*this,PC_INT_DOM);
  }
  // Post table propagator
  static ExecStatus post(Home home,
                         ViewArray<IntView>& x,
                         TupleSet t) {
    // All variables in the correct domain
    for (int i = x.size(); i--; ) {
      GECODE_ME_CHECK(x[i].gq(home, t.min()));
      GECODE_ME_CHECK(x[i].lq(home, t.max()));
    }
    // Only if there is something to propagate
    if (x.size() > 1)
      (void) new (home) CompactTable(home,x,t);
    return ES_OK;
  }
    
  // Copy constructor during cloning
  CompactTable(Space& home, bool share, CompactTable& p)
    : Propagator(home,share,p), t(p.t) {
    x.update(home,share,p.x);
  }

  // Create copy during cloning
  virtual Propagator* copy(Space& home, bool share) {
    return new (home) CompactTable(home,share,*this);
  }
    
  // Return cost (defined as cheap quadratic)
  virtual PropCost cost(const Space&, const ModEventDelta&) const {
    // TODO: ???
    return PropCost::quadratic(PropCost::LO,2*x.size());
  }
    
  // Perform propagation
  virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        
    //
    // This is what YOU have to add!
    //
        
  }
    
  // Dispose propagator and return its size
  virtual size_t dispose(Space& home) {
    x.cancel(home,*this,PC_INT_BND);
    // TODO: dispose t?
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }
};

// Post the table constraint
void extensional2(Home home, const IntVarArgs& x, const TupleSet& t) {
  // Never post a propagator in a failed space
  if (home.failed()) return;
  // Set up array of views for the coordinates
  ViewArray<IntView> vx(home,x);
  // If posting failed, fail space
  if (CompactTable::post(home,vx,t) != ES_OK)
    home.fail();
}
