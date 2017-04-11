#include <gecode/int.hh>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
//#include "bitset.hpp"
//#include "bitset-support.cpp"
#include "info-base.hpp"

//#define DEBUG

/** 
 * Threshold value for using hash table
 * Defined as domain-width / domain-size for each variable
 * (0->always hash, infinity->never hash)
 */
#define HASHH_THRESHOLD 2

typedef BitSet* Dom;

using namespace Gecode;
using namespace Gecode::Int;
using namespace std;

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

// Macros ARRAY and HASH are already defined on Linux
typedef enum {ARRAYY,HASHH} IndexType;

/**
 * Advisor
 **/
template<class View>
class CTAdvisor : public ViewAdvisor<View> {
public:
  /**
   * Shared handle for maintaining supports
   */
  class Supports : public SharedHandle {
  protected:
    class SupportsI : public SharedHandle::Object {
    public:
      /// The type of indexing
      IndexType type;
      /// Info object
      InfoBase* info;
      /// Default constructor (yields empty set)
      SupportsI(void) {}
      /// Copy \a s
      SupportsI(const SupportsI& s) 
        : type(s.type) {
        switch (type) {
        case ARRAYY: {
          info = new InfoArray(*((InfoArray *const)s.info));
          break;
        }
        case HASHH: {
          info = new InfoHash(*((InfoHash *const)s.info));
          break;
        }
        default:
          break;
        }
      }
      
      /// Initialise from \a s, \a init_min, \a nsupports, \a offset
      void init(const BitSet* s,
                int init_min, int max,
                int nsupports, int offset,
                int domain_offset, IndexType t, View x) {
        type = t;
        switch (type) {
        case ARRAYY:  {
          info = heap.alloc<InfoArray>(1);
          static_cast<InfoArray*>(info)->
            InfoArray::init(s,init_min,max,nsupports,offset,domain_offset,x);
          break;
        }
        case HASHH: {
          info = heap.alloc<InfoHash>(1);
          static_cast<InfoHash*>(info)->
            InfoHash::init(s,init_min,max,nsupports,offset,domain_offset,x);
          break;
        } 
        default:
          GECODE_NEVER;
          break;
        }
        
      }
      /// Copy function
      virtual Object* copy(void) const {
        return new SupportsI(*this);
      }
      /// Desctructor
      virtual ~SupportsI(void) {
        switch (type) {
        case ARRAYY: {
          static_cast<InfoArray*>(info)->~InfoArray();
          break;
        }
        case HASHH: {
          static_cast<InfoHash*>(info)->~InfoHash();
          break;
        }
        default:
          GECODE_NEVER;
          break;
        }

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
    Supports& operator =(const Supports& s) {
      return static_cast<SupportsI&>(SharedHandle::operator =(s));
    }
    /// [] operator
    BitSet operator [](unsigned int i) {
      const SupportsI* si = static_cast<SupportsI*>(object());
      return si->info->get_supports(i);
    }
    /// Initialise from parameters
    void init(BitSet* s, int min,int max,
              int nsupports, int offset,
              int domain_offset, IndexType type, View x) {
      static_cast<SupportsI*>(object())->
        init(s,min,max,nsupports,offset,domain_offset,type,x);
    }
    /// Update function
    void update(Space& home, bool share, SharedHandle& sh) {
      SharedHandle::update(home,share,sh);
    }
    /// Get the index for a value
    unsigned int row(int val) {
      return static_cast<SupportsI*>(object())->info->row(val);
    }
    /// Print
    void print() const {
      const SupportsI* si = static_cast<SupportsI*>(object());
      printf("nvals=%d, min=%d\n",si->nvals,si->min);
      for (int j = 0; j < si->nvals; j++) {
        si->info->get_supports(j).print();
      }
    }
  };
  /// Index of the view the advisor is assigned to
  int index;
  /// Tuple indices that are supports for the variable
  Supports supports;
  /// The word index of the last found support
  unsigned int* residues;
  /// Number of values
  unsigned int nvals;
  
  /// Constructor
  forceinline
  CTAdvisor(Space& home, Propagator& p,
            Council<CTAdvisor<View> >& c,
            View x0, int i,
            BitSet* s0,                    /*** Support information ***/
            unsigned int* res,             /*** Initial residues ***/
            int nsupports,
            int offset,                    /** Start index in s0 and res **/
            int dom_offset,                /** ts.min() **/
            IndexType type)
    : ViewAdvisor<View>(home,p,c,x0), index(i) {
    supports.init(s0,x0.min(),x0.max(),nsupports,offset,
                  dom_offset,type,x0);    
    // Initialise residues
    switch (type) {
    case ARRAYY: {
      // Sparse array
      nvals = x0.max() - x0.min() + 1;
      residues = home.alloc<unsigned int>(nvals);
      for (int i = 0; i < nvals; i++) {
        residues[i] = res[i + offset];
      }
      break;
    }
    case HASHH: {
      // Pack the residues tight        
      nvals = x0.size();
      residues = home.alloc<unsigned int>(nvals);
      int count = 0;
      int diff = x0.min();

      Int::ViewValues<View> it(x0);
      while (it()) {
        residues[count] = res[it.val() + offset - diff];
        ++count;
        ++it;
      }
      
      break;
    }
    default:
      GECODE_NEVER;
      break;
    }
  }
 
  /// Copy constructor
  forceinline
  CTAdvisor(Space& home, bool share, CTAdvisor<View>& a)
    : ViewAdvisor<View>(home,share,a),
    index(a.index), nvals(a.nvals) {
    supports.update(home,share,a.supports);
    residues = home.alloc<unsigned int>(nvals);
    for (int i = 0; i < nvals; i++)
      residues[i] = a.residues[i];
  }

  /// Access residue for value \a val
  forceinline unsigned int
  residue(int val) {
    //DEBUG_PRINT(("residue[%d] = %d\n",supports.row(val),residues[supports.row(val)]));
    return residues[supports.row(val)];
  }

  /// Set residue for value \a val
  forceinline void
  set_residue(int val, int r) {
    residues[supports.row(val)] = r;
  }
    
  /// Dispose function. TODO: dispose shared handle
  forceinline void
  dispose(Space& home, Council<CTAdvisor<View> >& c) {
    //DEBUG_PRINT(("Advisor %d disposed\n",index));
    // TODO: call destructor for supports?
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
    DEBUG_PRINT(("********************\n"));
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
#ifdef DEBUG
    validTuples.print();
#endif // DEBUG

    // Schedule in case no advisors have been posted
    View::schedule(home,*this,Int::ME_INT_VAL);
    DEBUG_PRINT(("End constructor\n"));
  }

  forceinline unsigned int
  init_supports(Home home, TupleSet ts) {
    static int ts_min = ts.min();
    
    Region r(home);
    // Bitsets for O(1) access to domains
    Dom dom = r.alloc<BitSet>(x.size());
    init_dom(home,dom,ts_min,ts.max());

    // Allocate temporary supports and residues
    BitSet* supports = r.alloc<BitSet>(domsum);
    unsigned int* residues = r.alloc<unsigned int>(domsum);
    for (int i = 0; i < domsum; i++)
      supports[i].init(r,ts.tuples(),false);
    
    // Save initial minimum value and widths for indexing supports and residues
    int* min_vals = r.alloc<int>(x.size());
    int* offset = r.alloc<int>(x.size());
    for (int i = 0; i<x.size(); i++) {
      min_vals[i] = x[i].min();
      offset[i] = i != 0 ? offset[i-1] + x[i-1].width() : 0;
    }

    int support_cnt = 0;
    int bpb = BitSet::get_bpb(); /** Bits per base in bitsets **/
    
    // Look for supports and set correct bits in supports
    for (int i = 0; i < ts.tuples(); i++) {
      bool supported = true;
      for (int j = ts.arity(); j--; ) {
        if (!dom[j].get(ts[i][j] - ts_min)) {
          supported = false;
          break;
        } 
      }
      if (supported) {
        // Set tuple as valid and save word index in residue
        for (int j = ts.arity(); j--; ) {
          int val = ts[i][j];
          unsigned int row = offset[j] + val - min_vals[j];
          supports[row].set(support_cnt);
          residues[row] = support_cnt / bpb;
        }
        support_cnt++;
      }
    }
    
    Support::StaticStack<int,Region> nq(r,domsum);

    // Remove values corresponding to empty rows and post advisors
    for (int i = x.size(); i--; ) {
      Int::ViewValues<View> it(x[i]);
      while (it()) {
        unsigned int row = offset[i] + it.val() - min_vals[i];
        if (supports[row].none()) {
          // Push to remove-stack and clear bit in domain
          DEBUG_PRINT(("Remove %d from variable %d\n", it.val(), i));
          nq.push(it.val());
        }
        ++it;
      }

      while (!nq.empty()) {
        GECODE_ME_CHECK(x[i].nq(home,nq.pop()));
      }

      if (!x[i].assigned()) {
        
        // Decide whether to use an array or a hash table
        double sparseness = x[i].width() / x[i].size();
        IndexType type = ARRAYY;
        if (sparseness >= HASHH_THRESHOLD)
          type = HASHH;

        // To shift the offset
        int diff = x[i].min() - min_vals[i];

        DEBUG_PRINT(("Offset %d: %d + %d = %d\n",i,offset[i],diff,offset[i]+diff));
                  
        (void) new (home) CTAdvisor<View>(home,*this,c,x[i],i,
                                          supports,
                                          residues,
                                          support_cnt,
                                          offset[i] + diff,
                                          ts.min(),
                                          type);
      } 
      
    }
    
    //home.notice(*this,AP_DISPOSE);
    return support_cnt;
  }

  forceinline void
  init_dom(Space& home, Dom dom, int min, int max) {
    unsigned int domsize = static_cast<unsigned int>(max - min + 1);
    for (int i = 0; i < x.size(); i++) {
      dom[i].init(home, domsize, false);
      Int::ViewValues<View> it(x[i]);
      while(it()) {
        dom[i].set(static_cast<unsigned int>(it.val() - min));
        ++it;
      }
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
    // Update views and advisors
    x.update(home,share,p.x);
    c.update(home,share,p.c);
  }
  
  // Create copy during cloning
  forceinline virtual Propagator*
  copy(Space& home, bool share) {
    return new (home) CompactTable(home,share,*this);
  }
    
  // Return cost
  forceinline virtual PropCost
  cost(const Space&, const ModEventDelta&) const {
    // Expensive linear ?
    return PropCost::linear(PropCost::HI,x.size());
  }

  forceinline virtual void
  reschedule(Space& home) {
    View::schedule(home,*this,ME_INT_DOM);
  }
  
  // Perform propagation
  forceinline virtual ExecStatus
  propagate(Space& home, const ModEventDelta&) {
    status = PROPAGATING;

    if (validTuples.is_empty()) {
      DEBUG_PRINT(("FAIL\n"));
      return ES_FAILED;
    }

    ExecStatus msg = filterDomains(home);

    status = NOT_PROPAGATING;
    return msg;
  }

  forceinline bool
  updateTable(CTAdvisor<View> a) {
    validTuples.clear_mask();
    Int::ViewValues<View> it(a.view());
    while (it()) {
      validTuples.add_to_mask(a.supports[it.val()]);
      ++it;
    }
    return validTuples.intersect_with_mask();
  }

  forceinline virtual ExecStatus
  advise(Space& home, Advisor& a0, const Delta& d) {
    CTAdvisor<View> a = static_cast<CTAdvisor<View>&>(a0);

    /** Do not schedule if propagator is performing propagation,
     *  and dispose if assigned
     **/
    if (status == PROPAGATING)
      return a.view().assigned() ? home.ES_FIX_DISPOSE(c,a)
        : ES_FIX;
        
    bool diff = updateTable(a);

    // Do not fail a disabled propagator
    if (validTuples.is_empty())
      return disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;

    // Schedule propagator and dispose if assigned
    if (diff)
      return a.view().assigned() ? home.ES_NOFIX_DISPOSE(c,a)
        : ES_NOFIX;

    // Fixpoint
    return a.view().assigned() ? home.ES_FIX_DISPOSE(c,a) : ES_FIX;
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

      if (a.view().assigned())
        continue;

      Int::ViewValues<View> it(x[i]);
      while (it()) {
        int index = a.residue(it.val());
        Support::BitSetData w = validTuples.a(a.supports[it.val()],index);
        
        if (w.none()) {
          index = validTuples.intersect_index(a.supports[it.val()]);

          if (index != -1) {
            // Save residue
            a.set_residue(i,index);
          } else {
            // Value not supported
            nq.push(it.val());
          }
        }
        ++it;
      }
      while (!nq.empty()) {
        int val = nq.pop();
        x[i].nq(home,val);
      }
        
      if (!x[i].assigned()) {
        count_non_assigned++;
        //--unassigned;
      } else {
        //DEBUG_PRINT(("Variable %d assigned in filterDomains\n",i));
      }
    }

    // Subsume if there is at most one non-assigned variable
    return count_non_assigned <= 1 ? home.ES_SUBSUMED(*this) : ES_FIX;
  }
  
  // Dispose propagator and return its size
  forceinline virtual size_t
  dispose(Space& home) {
    //x.cancel(home,*this,PC_INT_DOM);
    // TODO: dispose t?
    //home.ignore(*this,AP_DISPOSE);
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
