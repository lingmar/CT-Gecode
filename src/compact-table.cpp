#include <gecode/int.hh>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
//#include "bitset.hpp"
#include "bitset-support.cpp"


#define SHARED 1
#define MAX_FMT_SIZE 4096
#define PRINT
//#define DEBUG

typedef BitSet* Dom;

using namespace Gecode;
using namespace Gecode::Int;
using namespace std;

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif
  
/**
 * Advisor
 * Like a ViewAdvisor but with index of the view saved
 **/
template<class View>
class CTAdvisor : public ViewAdvisor<View> {
public:
  /// Class for managing supports
  class Supports : public SharedHandle {
  protected:
    class SupportsI : public SharedHandle::Object {
    public:
      /// Support bits (valid tuple indices)
      BitSet* supports;
      /// Residues
      unsigned int* residues;
      /// Initial minimum value for x
      int min;
      /// Number of values
      int nvals;
      /// Default constructor (yields empty set)
      SupportsI(void) {}
      /// Copy \a s
      SupportsI(const SupportsI& s) :
        min(s.min), nvals(s.nvals) {
        DEBUG_PRINT(("Copy SupportsI"));
        supports = heap.alloc<BitSet>(nvals);
        residues = heap.alloc<unsigned int>(nvals);
        for (int i = 0; i < nvals; i++) {
          supports[i].init(heap,s.supports[i].size());
          supports[i].copy(s.supports[i].size(), s.supports[i]);
          residues[i] = s.residues[i];
        }
      }
      /// Initialise from \a s, \a res, \a init_min, \a nsupports, \a offset 
      void init(const BitSet* s, const unsigned int* res,
                int init_min, int max,
                int nsupports, int offset) {
        // Number of bitsets
        nvals = static_cast<unsigned int>(max - init_min + 1);
        DEBUG_PRINT(("init supports, nsupports = %d, nvals = %d\n",
                     nsupports,nvals));

        // Allocate memory and initialise
        supports = heap.alloc<BitSet>(nvals);
        residues = heap.alloc<unsigned int>(nvals);
        for (int i = 0; i < nvals; i++) {
          assert(nsupports <= s[offset + i].size());
          supports[i].init(heap,nsupports);
          supports[i].copy(nsupports,s[i + offset]);

          residues[i] = res[i + offset];
        }
        min = init_min;
      }
      /// Copy function
      virtual Object* copy(void) const {
        return new SupportsI(*this);
      }
      /// Desctructor
      virtual ~SupportsI(void) {
        heap.rfree(residues, nvals);
        heap.rfree(supports, nvals);
      }
    };
  public:
    /// Default constructor
    Supports(void)
      : SharedHandle(new SupportsI()) {}
    /// Copy \a s
    Supports(const Supports& s)
      : SharedHandle(s) {
    }
    /// Assignment operator
    SharedSupports& operator =(const Supports& s) {
      return static_cast<SupportsI&>(SharedHandle::operator =(s));
    }
    /// [] operator
    BitSet operator [](unsigned int i) {
      const SupportsI* si = static_cast<SupportsI*>(object());
      return si->supports[i - si->min];
    }
    /// Initialise from parameters (only bit-set is deep-copied)
    void init(BitSet* s, unsigned int* residues,
              int min,int max, int nsupports,int offset) {
      static_cast<SupportsI*>(object())->init(s,residues,min,max,nsupports,offset);
    }
    /// Update function
    void update(Space& home, bool share, SharedHandle& sh) {
      SharedHandle::update(home,share,sh);
    }
    /// Get the residue for value \a v
    unsigned int residue(int v) const {
      const SupportsI* si = static_cast<SupportsI*>(object());
      return si->residues[v - si->min];
    }
    /// Set residue for value \a v to \a i
    void set_residue(int v, int i) {
      const SupportsI* si = static_cast<SupportsI*>(object());
      si->residues[v - si->min] = i;
    }
    /// Print
    void print() const {
      const SupportsI* si = static_cast<SupportsI*>(object());
      printf("nvals=%d, min=%d\n",si->nvals,si->min);
      for (int j = 0; j < si->nvals; j++) {
        si->supports[j].print();
      }
    }
  };
  /// Index of the view
  int index;
  /// Tuple indices that are supports for the variable
  Supports supports;
  /// Constructor
  forceinline
  CTAdvisor(Space& home, Propagator& p,
            Council<CTAdvisor<View> >& c,
            View x0, int i, BitSet* s0,
            unsigned int* residues, int min,
            int nsupports, int start_idx)
    : ViewAdvisor<View>(home,p,c,x0), index(i) {
    supports.init(s0,residues,min,x0.max(),nsupports,start_idx);
  }
 
  /// Copy constructor
  forceinline
  CTAdvisor(Space& home, bool share, CTAdvisor<View>& a)
    : ViewAdvisor<View>(home,share,a),
    index(a.index) {
    supports.update(home,share,a.supports);
  }
  
  /// Dispose function
  forceinline void
  dispose(Space& home, Council<CTAdvisor<View> >& c) {
    DEBUG_PRINT(("Advisor %d disposed\n",index));
    ViewAdvisor<View>::dispose(home,c);
  }
};

/**
 * The CT propagator
 **/
template<class View>
class CompactTable : public Propagator {
protected:
  enum Status {NOT_PROPAGATING,PROPAGATING};
  /// Whether propagator is done propagating
  Status status;
  /// The variables
  ViewArray<View> x;
  /// Council of advisors
  Council<CTAdvisor<View> > c;
  // The table with possible combinations of values
  SparseBitSet<Space&> validTuples;
  // Sum of domain sizes
  int domsum;
  
public:

  // Post table propagator
  forceinline static ExecStatus
  post(Home home, ViewArray<View>& x, TupleSet t) {
    // All variables in the correct domain
    for (int i = x.size(); i--; ) {
      GECODE_ME_CHECK(x[i].gq(home, t.min()));
      GECODE_ME_CHECK(x[i].lq(home, t.max()));
    }
    (void) new (home) CompactTable(home,x,t);
    if (home.failed()) {
      return ES_FAILED;
    }
    return ES_OK;
  }
  
  // Create propagator and initialize
  forceinline
  CompactTable(Home home,
               ViewArray<View>& x0,
               TupleSet t0)
    : Propagator(home), x(x0), c(home),
      validTuples(static_cast<Space&>(home)),
      status(NOT_PROPAGATING)
  {
    DEBUG_PRINT(("Start constructor\n"));
    // Calculate domain sum
    domsum = 0;
    int domsize = 0;
    for (int i = 0; i < x.size(); i++) {
      domsum += x[i].width();
      domsize += x[i].size();
    }
    DEBUG_PRINT(("domsum: %d, domsize: %d, ntuples: %d\n", domsum, domsize, t0.tuples()));
    
    // Initialise supports
    int nsupports = init_supports(home, t0);
    DEBUG_PRINT(("nsupports=%d\n",nsupports));
    
    if (nsupports <= 0) {
      home.fail();
      return;
    } 
    
    // Initialise validTupels with nsupports bit set
    validTuples.init(t0.tuples(), nsupports);
    DEBUG_PRINT(("End constructor\n"));

    // Schedule in case no advisors have been posted
    View::schedule(home,*this,Int::ME_INT_VAL);
  }

  forceinline unsigned int
  init_supports(Home home, TupleSet ts) {

    Region region(home);
    // Bitset for O(1) access to domains
    Dom dom = region.alloc<BitSet>(x.size());
    init_dom(home,dom,ts.min(),ts.max());

    int support_cnt = 0;
    int bpb = BitSet::get_bpb();

    /// Allocate BitSets and residues
    BitSet* supports = region.alloc<BitSet>(domsum);
    unsigned int* residues = region.alloc<unsigned int>(domsum);
    for (int i = 0; i < domsum; i++) {
      supports[i].Support::BitSetBase::init(region,ts.tuples());
    }
    
    // Save initial minimum value and widths for indexing
    int* min_vals = region.alloc<int>(x.size());
    int* offset = region.alloc<int>(x.size());
    for (int i = 0; i<x.size(); i++) {
      min_vals[i] = x[i].min();
      offset[i] = i != 0 ? offset[i-1] + x[i-1].width() : 0;
    }
        
    // Look for supports and set correct bits in supports
    for (int i = 0; i < ts.tuples(); i++) {
      bool supported = true;
      for (int j = ts.arity() - 1; j >= 0; j--) {
        if (!dom[j].get(ts[i][j] - ts.min())) {
          supported = false;
          break;
        } 
      }
      if (supported) {
        // Set tuple as valid and save residue
        for (int var = 0; var < ts.arity(); var++) {
          int val = ts[i][var];
          unsigned int row = offset[var] + val - min_vals[var];
          supports[row].set(support_cnt);
          residues[row] = support_cnt / bpb;
        }
        support_cnt++;
      }
    }
    
    Region r(home);
    Support::StaticStack<int,Region> nq(r,domsum);

    // Remove values corresponding to empty rows
    for (int i = 0; i < x.size(); i++) {
      Int::ViewValues<View> it(x[i]);
      while (it()) {
        unsigned int row = offset[i] + it.val() - min_vals[i];
        if (supports[row].none()) {
          nq.push(it.val());
        }
        ++it;
      }
      while (!nq.empty()) {
        GECODE_ME_CHECK(x[i].nq(home,nq.pop()));
      }
    }

    // Post advisors
    for (int i = x.size(); i--; ) {
      if (!x[i].assigned()) {
        DEBUG_PRINT(("%d\n",(supports + offset[i])[0].size()));
        (void) new (home) CTAdvisor<View>(home,*this,c,x[i],i,
                                          supports,
                                          residues,
                                          min_vals[i],
                                          support_cnt,
                                          offset[i]);
      } 
    }
        
    return support_cnt;
  }

  forceinline void
  init_dom(Space& home, Dom dom, int min, int max) {
    unsigned int domsize = static_cast<unsigned int>(max - min + 1);
    //cout << "max,min = " << max << "," << min << endl;
    //cout << "domsize=" << domsize << endl;
    for (int i = 0; i < x.size(); i++) {
      dom[i].Support::BitSetBase::init(home, domsize, false);
      Int::ViewValues<View> it(x[i]);
      while(it()) {
        dom[i].set(static_cast<unsigned int>(it.val() - min));
        ++it;
      }
      //dom[i].print();
    }
  }
  
  // Copy constructor during cloning
  forceinline
  CompactTable(Space& home, bool share, CompactTable& p)
    : Propagator(home,share,p),
      validTuples(home, p.validTuples),
      domsum(p.domsum),
      status(p.status)
  {
    DEBUG_PRINT(("Copy constructor,share=%d\n",share));
    x.update(home,share,p.x);
    c.update(home,share,p.c);
    DEBUG_PRINT(("End copy constructor\n"));
  }
  
  // Create copy during cloning
  forceinline virtual Propagator*
  copy(Space& home, bool share) {
    return new (home) CompactTable(home,share,*this);
  }
    
  // Return cost
  forceinline virtual PropCost
  cost(const Space&, const ModEventDelta&) const {
    // Expensive linear
    return PropCost::linear(PropCost::HI,x.size());
  }

  forceinline virtual void
  reschedule(Space& home) {
    View::schedule(home,*this, ME_INT_DOM);
  }
  
  // Perform propagation
  forceinline virtual ExecStatus
  propagate(Space& home, const ModEventDelta&) {
    DEBUG_PRINT(("Propagate\n"));

    status = PROPAGATING;

    if (validTuples.is_empty()) {
      return ES_FAILED;
    }

    ExecStatus msg = filterDomains(home);

    status = NOT_PROPAGATING;
    DEBUG_PRINT(("End propagate\n"));
    return msg;
  }

  forceinline void
  updateTable(CTAdvisor<View> a) {
    DEBUG_PRINT(("Update table %d\n", a.index));
    validTuples.clear_mask();
    Int::ViewValues<View> it(a.view());
    while (it()) {
      validTuples.add_to_mask(a.supports[it.val()]);
      ++it;
    }
    validTuples.intersect_with_mask();
  }

  forceinline virtual ExecStatus
  advise(Space& home, Advisor& a0, const Delta& d) {
    CTAdvisor<View> a = static_cast<CTAdvisor<View>&>(a0);

    DEBUG_PRINT(("Advise %d\n", a.index));
    DEBUG_PRINT(("Modevent: %d\n",a.view().modevent(d)));
    DEBUG_PRINT(("Domain size: %d\n",a.view().size()));
        
    // Do not schedule if propagator is performing propagation,
    // dispose if assigned
    if (status == PROPAGATING)
      return a.view().assigned() ? home.ES_FIX_DISPOSE(c,a)
        : ES_FIX;
    
    updateTable(a);
    if (validTuples.is_empty())
      return disabled() ? home.ES_FIX_DISPOSE(c,a) : ES_FAILED;

    // Schedule propagator and dispose if assigned
    DEBUG_PRINT(("Scheduling propagator %d\n",a.index));
    return a.view().assigned() ? home.ES_NOFIX_DISPOSE(c,a)
      : ES_NOFIX;
  }
  
  forceinline ExecStatus
  filterDomains(Space& home) {
    int count_non_assigned = 0;
    Region r(home);
    Support::StaticStack<int,Region> nq(r,domsum);
    // Only consider unassigned variables
    for (Advisors<CTAdvisor<View> > a0(c); a0(); ++a0) {
      CTAdvisor<View> a = a0.advisor();
      int i = a.index;

      if (a.view().assigned()) {
        DEBUG_PRINT(("Variable %d is assigned\n",i));
        continue;
      }

      DEBUG_PRINT(("Advisor for %d with domain size %d\n", i, x[i].size()));
      Int::ViewValues<View> it(x[i]);
      while (it()) {
        int index = a.supports.residue(it.val());

        // performing validTuples[index] & supports[x,a][index]
        Support::BitSetData w = validTuples.a(a.supports[it.val()],index);
        
        if (w.none()) {
          index = validTuples.intersect_index(a.supports[it.val()]);
        
          if (index != -1) {
            // Save residue
            a.supports.set_residue(i,it.val());
          } else {
            // Value not supported
            nq.push(it.val());
          }
        }
        ++it;
      }
      while (!nq.empty()) {
        int val = nq.pop();
        DEBUG_PRINT(("Remove value %d from variable %d\n",
                     val, i));
        x[i].nq(home,val);
      }
        
      if (!x[i].assigned()) {
        count_non_assigned++;
        //--unassigned;
      } else {
        DEBUG_PRINT(("Variable %d assigned in filterDomains\n",i));
      }
    }

    // Subsume if there is at most one non-assigned variable
    return count_non_assigned <= 1 ? home.ES_SUBSUMED(*this) : ES_FIX;
    //return count_non_assigned <= 1 ? home.ES_SUBSUMED(*this) : ES_FIX;
  }
  
  // Dispose propagator and return its size
  forceinline virtual size_t
  dispose(Space& home) {
    //x.cancel(home,*this,PC_INT_DOM);
    // TODO: dispose t?
    c.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

};


// Post the table constraint
namespace Gecode {
  
  forceinline void
  extensional2(Home home, const IntVarArgs& x, const TupleSet& t) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional2");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional2");
    // Never post a propagator in a failed space
    GECODE_POST;
    
    if (t.tuples()==0) {
      if (x.size()!=0) {
        home.fail();
      }
      return;
    }
    // Construct view array
    ViewArray<IntView> vx(home,x);
    GECODE_ES_FAIL(CompactTable<IntView>::post(home,vx,t));
  }

  forceinline void
  extensional2(Home home, const BoolVarArgs& x, const TupleSet& t) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional2");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional2");
    // Never post a propagator in a failed space
    GECODE_POST;
    if (t.tuples()==0) {
      if (x.size()!=0) {
        home.fail();
      }
      return;
    }
    // Construct view array
    ViewArray<BoolView> vx(home,x);
    GECODE_ES_FAIL(CompactTable<BoolView>::post(home,vx,t));
  }
}
