#include <gecode/int.hh>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
//#include "bitset.hpp"
//#include "bitset-support.cpp"
#include "info-base.hpp"

//#define DELTA

//#define DEBUG

/** 
 * Threshold value for using hash table
 * Defined as domain-width / domain-size for each variable
 * (0->always hash, infinity->never hash)
 */
#define HASHH_THRESHOLD 3

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
          info = new InfoHash(*((InfoHash *const)s .info));
          break;
        }
        default:
          break;
        }
      }
      
      /// Initialise from \a s, \a init_min, \a nsupports, \a offset
      void init(const BitSet* s,
                int nsupports, int offset,
                IndexType t, View x) {
        type = t;
        switch (type) {
        case ARRAYY:  {
          info = heap.alloc<InfoArray>(1);
          static_cast<InfoArray*>(info)->
            InfoArray::init(s,nsupports,offset,x);
          break;
        }
        case HASHH: {
          info = heap.alloc<InfoHash>(1);
          static_cast<InfoHash*>(info)->
            InfoHash::init(s,nsupports,offset,x);
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
        heap.rfree(info);
      }
    };
  public:

    Supports(void) {}

    Supports(BitSet* s, int nsupports, int offset,
             IndexType type, View x)
      : SharedHandle(new SupportsI()) {
        init(s,nsupports,offset,type,x);
    }

    /// Copy \a s
    Supports(const Supports& s)
      : SharedHandle(s) {
    }
    /// Assignment operator
    Supports& operator =(const Supports& s) {
      return static_cast<SupportsI&>(SharedHandle::operator =(s));
    }
    /// [] operator
    const BitSet& operator [](unsigned int i) {
      const SupportsI* si = static_cast<SupportsI*>(object());
      return si->info->get_supports(i);
    }
    /// () operator returns the actual row
    const BitSet& operator ()(unsigned int i) {
      const SupportsI* si = static_cast<SupportsI*>(object());
      return si->info->get_supports_raw(i);
    }
    /// Initialise from parameters
    void init(BitSet* s,
              int nsupports, int offset,
              IndexType type, View x) {
      
      static_cast<SupportsI*>(object())->
        init(s,nsupports,offset,type,x);
    }
    /// Update function
    void update(Space& home, bool share, SharedHandle& sh) {
      SharedHandle::update(home,share,sh);
    }
    /// Get the index for a value
    int row(int val) {
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
  /// Tuple indices that are supports for the variable (shared handle)
  Supports supports;
  /// The word index of the last found support for each value
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
            IndexType type)
    : ViewAdvisor<View>(home,p,c,x0),
    index(i),
    supports(s0,nsupports,offset,type,x0)
  {
#ifdef DEBUG
    cout << i << ": " << x0 << endl;    
#endif // DEBUG

    // Initialise residues
    switch (type) {
    case ARRAYY: {
      // Sparse array
      nvals = x0.max() - x0.min() + 1;
      residues = home.alloc<unsigned int>(nvals);
      for (unsigned int i = 0; i < nvals; i++)
        residues[i] = res[i + offset];
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
    index(a.index), nvals(a.nvals)
  {
    // Update shared handle
    supports.update(home,share,a.supports);
    // Copy residues
    residues = home.alloc<unsigned int>(nvals);
    for (int i = nvals; i--; )
      residues[i] = a.residues[i];
  }

  /// Access residue for value \a val
  forceinline unsigned int
  residue(int val) {
    return residues[supports.row(val)];
  }

  /// Set residue for value \a val
  forceinline void
  set_residue(int val, int r) {
    residues[supports.row(val)] = r;
  }
    
  forceinline void
  dispose(Space& home, Council<CTAdvisor<View> >& c) {
    //home.ignore(*this,AP_DISPOSE);
    (void) supports.~Supports();
    (void) ViewAdvisor<View>::dispose(home,c);
  }
};

/**
 * The CT propagator
 **/
template<class View>
class CompactTable : public Propagator {
protected:
  enum Status {NOT_PROPAGATING,PROPAGATING};
  /// Council of advisors
  Council<CTAdvisor<View> > c;
  /// Whether propagator is propagating
  Status status;
  /// -2 if more than one touched var, -1 if none, else index of touched var
  int touched_var;
  /// The indices of valid tuples
  SparseBitSet<Space&> validTuples;
  /// Arity
  unsigned int arity;
  /// Number of unassigned variables
  unsigned int unassigned;
  /// Tuple-set
  TupleSet tupleSet;
  /// Largest domain size
  unsigned int max_dom_size;
  /// Whether it is the first time the advisor executes
  //  bool first;
  
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
    if (home.failed())
      return ES_FAILED;
    return ES_OK;
  }
  
  // Create propagator and initialize
  forceinline
  CompactTable(Home home,
               ViewArray<View>& x0,
               TupleSet t0)
    : Propagator(home), c(home),
      status(NOT_PROPAGATING),
      touched_var(-1),
      validTuples(static_cast<Space&>(home)),
      arity(x0.size()),
      unassigned(x0.size())
  {
    DEBUG_PRINT(("******* Constructor *******\n"));
    // Initialise supports and post advisors
    int nsupports = init_supports(home, t0, x0);
    DEBUG_PRINT(("nsupports=%d\n",nsupports));
    
    if (nsupports <= 0) {
      home.fail();
      return;
    } 
    
    // Initialise validTupels with nsupports bit set
    validTuples.init(nsupports);

    // Because we use heap allocated data
    home.notice(*this,AP_DISPOSE);
    
    // Schedule in case no advisors have been posted
    if (unassigned == 0)
      View::schedule(home,*this,Int::ME_INT_VAL);
  }

  forceinline unsigned int
  init_supports(Home home, TupleSet ts, ViewArray<View>& x) {
    // Find maximum domain size and total domain width
    max_dom_size = 1;
    unsigned int domsum = 0;
    for (int i = x.size(); i--; ) {
      domsum += x[i].width();
      if (x[i].size() > max_dom_size)
        max_dom_size = x[i].size();
    }

    Region r(home);
    // Temporary array to create tupleset
    int* tuple = r.alloc<int>(x.size());
    // Allocate temporary supports and residues
    BitSet* supports = r.alloc<BitSet>(domsum);
    unsigned int* residues = r.alloc<unsigned int>(domsum);
    
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
        if (!x[j].in(ts[i][j])) {
          supported = false;
          break;
        } 
      }
      if (supported) {
        // Set tuple as valid and save word index in residue
        for (int j = ts.arity(); j--; ) {
          int val = ts[i][j];
          tuple[j] = val;
          unsigned int row = offset[j] + val - min_vals[j];

          if (supports[row].empty()) // Initialise in case not done
            supports[row].init(r,ts.tuples(),false);

          supports[row].set(support_cnt);
          residues[row] = support_cnt / bpb;
        }
        support_cnt++;
        tupleSet.add(IntArgs(x.size(),tuple));
      }
    }

    int* nq = r.alloc<int>(max_dom_size);
    int nremoves;
    max_dom_size = 1; // Reset maximal domain size
    
    // Remove values corresponding to empty rows 
    for (int i = x.size(); i--; ) {
      nremoves = 0;

      Int::ViewValues<View> it(x[i]);
      while (it()) {
        unsigned int row = offset[i] + it.val() - min_vals[i];
        if (supports[row].size() == 0) {
          nq[nremoves++] = it.val();
          DEBUG_PRINT(("Pruned %d from variable %d\n",it.val(),i));
        }
        ++it;
      }

      Iter::Values::Array r(nq,nremoves);
      GECODE_ME_CHECK(x[i].minus_v(home,r,false));
      if (x[i].size() > max_dom_size)
        max_dom_size = x[i].size();
    }

    // Post advisors
    for (int i = x.size(); i--; ) {
      if (!x[i].assigned()) {
        // Decide whether to use an array or a hash table
        double sparseness = x[i].width() / x[i].size();
        IndexType type = sparseness < HASHH_THRESHOLD ? ARRAYY : HASHH;

        // To shift the offset
        int diff = x[i].min() - min_vals[i];

        (void) new (home) CTAdvisor<View>(home,*this,c,x[i],i,
                                          supports,
                                          residues,
                                          support_cnt,
                                          offset[i] + diff,
                                          type);
      } else
        unassigned--;      
    }
    tupleSet.finalize();
    return support_cnt;
  }

  // Copy constructor during cloning
  forceinline
  CompactTable(Space& home, bool share, CompactTable& p)
    : Propagator(home,share,p),
      status(p.status),
      touched_var(p.touched_var),
      validTuples(home, p.validTuples),
      arity(p.arity),
      unassigned(p.unassigned),
      max_dom_size(p.max_dom_size)
  {
    // Update views and advisors
    c.update(home,share,p.c);
    tupleSet.update(home,share,p.tupleSet);
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
    return PropCost::linear(PropCost::HI,arity);
  }

  forceinline virtual void
  reschedule(Space& home) {
    View::schedule(home,*this,ME_INT_DOM);
  }
  
  // Perform propagation
  forceinline virtual ExecStatus
  propagate(Space& home, const ModEventDelta&) {
    status = PROPAGATING;
    
    if (validTuples.is_empty())
      return ES_FAILED;

    ExecStatus msg;
    if (validTuples.one()) 
      msg = fixDomains(home);
    else
      msg = filterDomains(home);

    status = NOT_PROPAGATING;
    return msg;
  }

  forceinline bool
  incremental_update(CTAdvisor<View> a, Space& home) {
    // Collect all tuples to be kept in a temporary mask
    Region r(home);
    BitSet mask;
    mask.allocate(r,validTuples.size());
    validTuples.clear_mask(mask);
    
    Int::ViewValues<View> it(a.view());
    while (it()) {
      assert(!a.supports[it.val()].empty());

      validTuples.add_to_mask(a.supports[it.val()],mask);
      ++it;
    }
    return validTuples.intersect_with_mask(mask);
  }
  
  forceinline virtual ExecStatus
  advise(Space& home, Advisor& a0, const Delta& d) {
    // Do not schedule if propagator is performing propagation,
    // and dispose if assigned
    if (status == PROPAGATING)
      return ES_FIX; // Advisor is disposed in propagate() if assigned

    CTAdvisor<View> a = static_cast<CTAdvisor<View>&>(a0);
    ModEvent me = View::modevent(d);
    View x = a.view();
    bool diff;
#ifdef DELTA
    if (me == ME_INT_VAL) { // Variable is assigned -- intersect with its value
      diff = validTuples.intersect_with_mask(a.supports[x.val()]);
    } else if (x.any(d)){ // No delta information -- do incremental update
      diff = incremental_update(a,home);
    } else { // Delta information available -- let's compare the size of
             // the domain with the size of delta to decide whether or not
             // to do reset-based or incremental update
      int min_rm = x.min(d);
      int max_rm = x.max(d);
      int min_row = a.supports.row(min_rm);
      int max_row = a.supports.row(max_rm);
      // Push min_row and max_row to closest corresponding tabulated values.
      // This happens if min_rm or max_rm were not in the domain of x
      // when the advisor was posted. Those values need not be considered since
      // we were at fixpoint when the advisor was posted.
      while (min_row == -1) // -1 means value is not tabulated
        min_row = a.supports.row(++min_rm);
      while (max_row == -1)
        max_row = a.supports.row(--max_rm);
      assert(max_row >= min_row);

      if (static_cast<unsigned int>(max_row - min_row + 1) <= x.size()) { // Delta is smaller 
                                               // Reset-based update
        // if (min_row == max_row) { // Only one value -- don't use a mask
        //   validTuples.nand(a.supports(min_row));
        // } else {
        // Region r(home);
        // BitSet mask;
        // mask.allocate(r,validTuples.size());
        // validTuples.clear_mask(mask);
        // for (int i = min_row; i <= max_row; i++) {
        //   if (!a.supports(i).empty()) { // empty if val not in initial domain
        //     validTuples.add_to_mask(a.supports(i),mask);
        //   }
        // }
        // validTuples.reverse_mask(mask);
        // diff = validTuples.intersect_with_mask(mask);

        diff = false;
        for (int i = min_row; i <= max_row; i++) {
          if (!a.supports(i).empty() && validTuples.nand(a.supports(i))) {
            diff = true;
          }
        }
      } else { // Domain size smaller than delta, incremental update
        diff = incremental_update(a,home);
      }
    } 
#else
    diff = incremental_update(a,home);
#endif // DELTA
    
    

    // Do not fail a disabled propagator
    if (validTuples.is_empty())
      return disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;

    // Schedule propagator and dispose if assigned
    if (diff) {
      if (touched_var == -1) // no touched variable yet!
        touched_var = a.index;
      else if (touched_var != a.index) // some other variable is touched
        touched_var = -2;
      
      if (a.view().assigned()) {
        unassigned--;
        return home.ES_NOFIX_DISPOSE(c,a);
      }
      return ES_NOFIX;
    }

    if (a.view().assigned()) {
      unassigned--;
      return home.ES_FIX_DISPOSE(c,a);
    }
      
    return ES_FIX;
  }
  
  forceinline ExecStatus
  filterDomains(Space& home) {
    if (unassigned == 0) return home.ES_SUBSUMED(*this);
    // Count the number of scanned unassigned variables
    unsigned int count_unassigned = 0;
    // Array to collect values to remove
    Region r(home);
    int* nq = r.alloc<int>(max_dom_size);
    
    // Smallest possible domain size
    max_dom_size = 1;

    // Scan all values of all unassigned variables to see if they
    // are still supported.
    for (Advisors<CTAdvisor<View> > a0(c);
         a0() && count_unassigned < unassigned; // End if only assigned variables left
         ++a0) {
      CTAdvisor<View> a = a0.advisor();
      View v = a.view();
      int i = a.index;
      
      // No point filtering variable if it was the only modified variable
      if (touched_var == i) {
        touched_var = -1;
        if (v.size() > max_dom_size)
          max_dom_size = v.size();
        continue;
      }

      switch (v.size()) {
      case 1: {  // Variable assigned, nothing to be done.
        break;
      }
      case 2: { // Consider min and max values
        const int min_val = v.min();
        const int max_val = v.max();

        // Fix to max_val if min_val not supported
        if (!supported(a,min_val)) {
          v.eq(home,max_val);
          --unassigned;
          a.dispose(home,c);
          break;
        }

        // Fix to min_val if max_val not supported
        if (!supported(a,max_val)) {
          v.eq(home,min_val);
          --unassigned;
          a.dispose(home,c);
          break;
        }

        // Otherwise v is still unassigned
        count_unassigned++;
        break;
      } default:
        const int min_val = v.min();
        const int max_val = v.max();
        int new_min = min_val;
        int new_max = max_val;
        int nremoves = 0;
        
        // Iterate over single range if domain is an interval
        if (v.range()) {
          // Increase new_min to smallest supported value
          for (; new_min <= max_val; ++new_min) {
            if (supported(a,new_min))
              break;
          }
          v.gq(home,new_min);
          if (v.assigned()) {
            --unassigned;
            a.dispose(home,c);
            break;
          }
          // Decrease new_max to largest supported value
          for (; new_max >= new_min; --new_max) {
            if (supported(a,new_max))
              break;
          }
          v.lq(home,new_max);
          if (v.assigned()) {
            --unassigned;
            a.dispose(home,c);
            break;
          }
          // Filter out values in between min and max
          for (int val = new_min + 1; val < new_max; ++val) {
            if (!supported(a,val))
              nq[nremoves++] = val;
          }
          
        } else { // Domain is not a range
          new_min = Limits::max; // Escape value

          Int::ViewValues<View> it(v);
          while (it()) {
            if (!supported(a,it.val()))
              nq[nremoves++] = it.val();
            else {
              if (new_min == Limits::max) {
                new_min = it.val();
                nremoves = 0; // Will be covered by gq
              }
              new_max = it.val(); // Will collect the largest supported value
            }
            ++it;
          }
          // Perform bounds propagation first
          v.gq(home,new_min);
          if (v.assigned()) {
            --unassigned;
            a.dispose(home,c);
            break;
          }
          v.lq(home,new_max);
          if (v.assigned()) {
            --unassigned;
            a.dispose(home,c);
            break;
          }
          // Trim nremoves
          while (nremoves > 0 && nq[nremoves-1] > new_max)
            nremoves--;
        }
        // Domain propagation
        if (nremoves > 0) {
          Iter::Values::Array r(nq,nremoves);
          v.minus_v(home,r,false);
          if (v.assigned()) {
            --unassigned;
            a.dispose(home,c);
            break;
          }
        }
        
        if (v.size() > max_dom_size)
          max_dom_size = v.size();

        ++count_unassigned;
      }
    }
    // Subsume if there is at most one non-assigned variable
    return unassigned <= 1 ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  forceinline bool
  supported(CTAdvisor<View>& a, int val) {
    int index = a.residue(val);
    Support::BitSetData w = validTuples.a(a.supports[val],index);
        
    if (w.none()) {
      index = validTuples.intersect_index(a.supports[val]);
      if (index != -1) {
        a.set_residue(val,index);
        return true;
      }
      return false;
    }
    return true;
  }

  forceinline ExecStatus
  fixDomains(Space& home) {
    // Only one valid tuple left, so we can fix all vars to that tuple
    unsigned int tuple_index = validTuples.index_of_fixed();
    TupleSet::Tuple t = tupleSet[tuple_index];
    for (Advisors<CTAdvisor<View> > a0(c); a0(); ++a0) {
      CTAdvisor<View> a = a0.advisor();
      View v = a.view();
      v.eq(home,t[a.index]);
      a.dispose(home,c);
    }

    return home.ES_SUBSUMED(*this);
  }
  
  // Dispose propagator and return its size
  forceinline virtual size_t
  dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    c.dispose(home);
    (void) tupleSet.~TupleSet();
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
