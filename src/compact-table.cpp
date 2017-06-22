#include <gecode/int.hh>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
#include "info-base.hpp"

#define NOISY 0

//#define DEBUG

//#define LONG_FILTER
//#define FIX
#define DELTA

#define CLEAR_MASK

/** 
 * Threshold value for using hash table
 * Defined as domain-width / domain-size for each variable
 * (0->always hash, infinity->never hash)
 */
#define HASHH_THRESHOLD 3

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
      /// Initialise from parameters
      forceinline void
      init(const BitSet* s,
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
      /// Desctructor
      forceinline virtual
      ~SupportsI(void) {
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
    forceinline
    Supports(void) {}
    
    forceinline
    Supports(BitSet* s, int nsupports, int offset,
             IndexType type, View x)
      : SharedHandle(new SupportsI()) {
      static_cast<SupportsI*>(object())->
        init(s,nsupports,offset,type,x);
    }

    /// Copy \a s
    forceinline
    Supports(const Supports& s)
      : SharedHandle(s) {
    }
    forceinline const BitSet&
    operator [](unsigned int i) {
      const SupportsI* si = static_cast<SupportsI*>(object());
      return si->info->get_supports(i);
    }
    /// Initialise from parameters
    forceinline
    void init(BitSet* s,
              int nsupports, int offset,
              IndexType type, View x) {
      static_cast<SupportsI*>(object())->
        init(s,nsupports,offset,type,x);
    }
    /// Get the index for a value
    forceinline int
    row(int val) {
      return static_cast<SupportsI*>(object())->info->row(val);
    }

  };
  /// Index of the view the advisor is assigned to
  int index;
  /// Tuple indices that are supports for the variable (shared handle)
  Supports supports;
  /// The word index of the last found support for each value
  unsigned int* residues;

  int offset;
  
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
    offset(0),
    supports(s0,nsupports,offset,type,x0)
  {
    // Initialise residues
    switch (type) {
    case ARRAYY: {
      // Sparse array
      int nvals = x0.max() - x0.min() + 1;
      residues = home.alloc<unsigned int>(nvals);
      for (unsigned int i = 0; i < nvals; i++)
        residues[i] = res[i + offset];
      break;
    }
    case HASHH: {
      // Pack the residues tight
      int nvals = x0.size();
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
  CTAdvisor(Space& home, CTAdvisor<View>& a)
    : ViewAdvisor<View>(home,a),
    supports(a.supports)
  {
    View x = a.view();
    if (!x.assigned()) {
      index = a.index;
      // Copy residues
      const int min_row = supports.row(x.min()) - a.offset;
      const int max_row = supports.row(x.max()) - a.offset;
      offset = a.offset + min_row;
      residues = home.alloc<unsigned int>(x.width());
      int cnt = 0;
      for (int i = min_row; i <= max_row; i++)
        residues[cnt++] = a.residues[i];
    }
  }
    
  forceinline void
  dispose(Space& home, Council<CTAdvisor<View> >& c) {
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
#ifdef FIX
  /// Tuple-set
  TupleSet tupleSet;
#endif // FIX
  /// Number of unassigned variables
  unsigned int unassigned;
  /// -2 if more than one touched var, -1 if none, else index of the only touched var
  int touched_var;
  /// Whether propagator is propagating or not
  Status status;
  /// Largest domain size
  unsigned int max_dom_size;
  /// Arity
  unsigned int arity;
  // *** Sparse bit-set *** //
  /// Valid bits
  BitSet words;
  /// Maps the current index to its original index in words
  int* index;
  /// Equals to the number of non-zero words minus 1
  int limit;
  
private:
  /// Initialise sparse bit-set with space for \a s bits (only after call to default constructor)
  forceinline void
  init_sparse_bit_set(Space& home, unsigned int s) {
    words.init(home,s);
    int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
    limit = nwords - 1;
    index = home.alloc<int>(nwords);
    for (int i = limit+1; i--; )
      index[i] = i;
    // Set the first s nr of bits in words
    int start_bit = 0;
    int complete_words = s / BitSet::get_bpb();
    if (complete_words > 0) {
      start_bit = complete_words * BitSet::get_bpb() + 1;
      words.Gecode::Support::RawBitSetBase::clearall(start_bit - 1,true);
    }
    for (unsigned int i = start_bit; i < s; i++) {
      words.set(i);
    }

    assert(nzerowords());
    assert(limit >= 0);
    assert(nset() == s);
    assert(limit <= Support::BitSetData::data(words.size()) - 1);
  }
  /// Check if sparse bit set is empty
  forceinline bool
  is_empty() const {
    return limit == -1;
  }
  /// Clear the mask
  forceinline void
  clear_mask(BitSet& mask) {
    assert(limit >= 0);
    assert(nzerowords());
    BitSet::clear_to_limit(mask, static_cast<unsigned int>(limit));
  }
  forceinline void
  init_mask(const BitSet& a, const BitSet& b, BitSet& mask) {
    assert(limit >= 0);
    assert(nzerowords());
    BitSet::init_from_bs(mask,a,b,index,static_cast<unsigned int>(limit));
  }
  
  /// Add bits in \a b to \a mask
  forceinline void
  add_to_mask(const BitSet& b, BitSet& mask) const {
    assert(limit >= 0);
    assert(nzerowords());
    BitSet::or_by_map(mask, b, index, static_cast<unsigned int>(limit));
  }
  /// Intersect words with \a mask
  forceinline void
  intersect_with_mask(const BitSet& mask) {
    assert(limit >= 0);
    assert(nzerowords());
    BitSet::intersect_by_map(words,mask,index,&limit);
  }
  /// Intersect words with \a mask
  forceinline void
  intersect_with_mask_sparse_one(const BitSet& mask) {
    assert(limit >= 0);
    assert(nzerowords());
    BitSet::intersect_by_map_sparse(words,mask,index,&limit);
  }
  /// Intersect words with the or of \a and \a b
  forceinline void
  intersect_with_mask_sparse_two(const BitSet& a, const BitSet& b) {
    assert(limit >= 0);
    assert(nzerowords());
    BitSet::intersect_by_map_sparse_two(words,a,b,index,&limit);
  }

  /// Get the index of a non-zero intersect with \a b, or -1 if none exists
  forceinline int
  intersect_index(const BitSet& b, int max_index) const {
    assert(limit >= 0);
    assert(max_index >= 0);
    assert(max_index <= limit);
    assert(nzerowords());
    return BitSet::intersect_index_by_map(words,b,index,
                                          static_cast<unsigned int>(max_index));
    
  }
  /// Perform "nand" with \a b
  forceinline void
  nand_with_mask_one(const BitSet& b) {
    assert(limit >= 0);
    assert(nzerowords());
    BitSet::nand_by_map_one(words,b,index,&limit);
  }
  /// Perform "nand" with the "or" of \a a and \a b
  forceinline void
  nand_with_mask_two(const BitSet& a, const BitSet& b) {
    assert(limit >= 0);
    assert(nzerowords());
    BitSet::nand_by_map_two(words,a,b,index,&limit);
  }

  /// Test whether exactly one bit is set in words
  forceinline bool
  one() const {
    assert(limit >= 0);
    assert(nzerowords());
    return limit == 0 && words.one(0);
  }
  /// Get the index of the set bit (only after one() returns true)
  forceinline unsigned int
  index_of_fixed() const {
    assert(limit >= 0);
    assert(nzerowords());
    assert(one());
    // The word index is index[limit]
    // Bit index is word_index*bpb + bit_index
    unsigned int bit_index = words.getword(0).next();
    return index[0] * words.get_bpb() + bit_index;
  }
  // Debugging only
  int nset() {
    int count = 0;
    for (int i = 0; i <= limit; i++) {
      for (int j = 0; j < BitSet::get_bpb(); j++) {
        count += words.get(i*BitSet::get_bpb() + j);
        if (i*BitSet::get_bpb() + j == words.size() - 1) {
          break;
        }
      }
    }
    return count;
  }
  
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
      arity(x0.size()),
      unassigned(x0.size())
  {
    // Initialise supports and post advisors
    int nsupports = init_supports(home, t0, x0);
    
    if (nsupports <= 0) {
      home.fail();
      return;
    } 

    init_sparse_bit_set(home, nsupports);
    
    // Because we use heap allocated data in advisors
    home.notice(*this,AP_DISPOSE);
    
    // Schedule in case we can subsume
    if (unassigned <= 1)
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

    Region r;
#ifdef FIX
    // Temporary array to create tupleset
    int* tuple = r.alloc<int>(x.size());
#endif // FIX
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
    int bpb = BitSet::get_bpb(); // Bits per base (word) in bitsets
    
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
#ifdef FIX
          tuple[j] = val;
#endif // FIX
          unsigned int row = offset[j] + val - min_vals[j];

          if (supports[row].empty()) // Initialise in case not done
            supports[row].init(r,ts.tuples(),false);

          supports[row].set(support_cnt);
          // Save the index in words where a support is found for the value
          residues[row] = support_cnt / bpb;
        }
        support_cnt++;
#ifdef FIX
        tupleSet.add(IntArgs(x.size(),tuple));        
#endif // FIX
      }
    }

    int* nq = r.alloc<int>(max_dom_size);
    int nremoves;
    
    // Remove values corresponding to empty rows 
    for (int i = x.size(); i--; ) {
      nremoves = 0;

      Int::ViewValues<View> it(x[i]);
      while (it()) {
        unsigned int row = offset[i] + it.val() - min_vals[i];
        if (supports[row].size() == 0) {
          nq[nremoves++] = it.val();
        }
        ++it;
      }
      Iter::Values::Array r(nq,nremoves);
      GECODE_ME_CHECK(x[i].minus_v(home,r,false));
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
#ifdef FIX
    tupleSet.finalize();    
#endif // FIX
    return support_cnt;
  }

  // Copy constructor during cloning
  forceinline
  CompactTable(Space& home, CompactTable& p)
    : Propagator(home,p),
      status(NOT_PROPAGATING),
      touched_var(-1),
      arity(p.arity),
      unassigned(p.unassigned),
      max_dom_size(p.max_dom_size),
      limit(p.limit)
  {
    // Update advisors
    c.update(home,p.c);
#ifdef FIX
    tupleSet.update(home,share,p.tupleSet);
#endif // FIX
    index = home.alloc<int>(limit + 1);
    for (int i = limit+1; i--; )
      index[i] = p.index[i];
    unsigned int nbits = (limit + 1) * BitSet::get_bpb();
    words.allocate(home, nbits);
    words.copy(nbits, p.words);
    assert(limit <= Support::BitSetData::data(words.size()) - 1);
  }

  // Create copy during cloning
  forceinline virtual Actor*
  copy(Space& home) {
    return new (home) CompactTable(home,*this);
  }
    
  // Return cost
  forceinline virtual PropCost
  cost(const Space&, const ModEventDelta& med) const {
    // // Expensive linear ?
    // return PropCost::linear(PropCost::HI,arity);
    if (View::me(med) == ME_INT_VAL)
      return PropCost::quadratic(PropCost::HI,arity);
    else
      return PropCost::cubic(PropCost::HI,arity); 
  }

  forceinline virtual void
  reschedule(Space& home) {
    View::schedule(home,*this,ME_INT_DOM);
  }
  
  // Perform propagation
  forceinline virtual ExecStatus
  propagate(Space& home, const ModEventDelta&) {
    status = PROPAGATING;
    if (is_empty())
      return ES_FAILED;
    assert(nset() > 0);
    assert(nzerowords());
    assert(limit >= 0);
#ifdef FIX
    ExecStatus msg;
    if (one()) 
      msg = fixDomains(home);
    else
      msg = filterDomains(home);
#else
    ExecStatus msg = filterDomains(home);
#endif // FIX

    touched_var = -1;
    status = NOT_PROPAGATING;
    assert(limit >= 0);
    assert(nset() > 0);
    assert(nzerowords());
    return msg;
  }

  forceinline void
  reset_based_update(CTAdvisor<View>& a, Space& home) {
    assert(nzerowords());
    assert(limit >= 0);
    assert(a.view().size() >= 2);
    switch (a.view().size()) {
    case 2: {
      // Intersect with validTuples directly
      int row_min = a.supports.row(a.view().min());
      int row_max = a.supports.row(a.view().max());
      intersect_with_mask_sparse_two(a.supports[row_min],
                                     a.supports[row_max]);
      break;
    }
    default:
      // Collect all tuples to be kept in a temporary mask
      Region r;
      BitSet mask;
      mask.allocate(r,words.size());

      Int::ViewRanges<View> rngs(a.view());
      //cout << a.view() << endl;
    
#ifdef CLEAR_MASK
      clear_mask(mask);
      int cur, max, row;
      while (rngs()) {
        cur = rngs.min();
        max = rngs.max();
        row = a.supports.row(cur);
        while (cur <= max) {
          assert(a.view().in(cur));
          add_to_mask(a.supports[row],mask);
          ++cur;
          ++row;
        }
        ++rngs;
      }
#else
      assert(rngs());
      int cur = rngs.min();
      int max = rngs.max();
      int row = a.supports.row(cur);
      assert(row >= 0);
      // Find the support entries for the first two values,
      // to initialise the mask from
      const BitSet& first = a.supports[row];
      if (cur < max) { // First range has more than one value
        ++cur;
        ++row;
        assert(a.view().in(cur));
      } else { // First range has only one value
        assert(rngs());
        ++rngs;
        cur = rngs.min();
        max = rngs.max();
        row = a.supports.row(cur);
        assert(row >= 0);
        assert(a.view().in(cur));
      }  
      // Initailise mask
      init_mask(first,a.supports[row],mask);

      // Flush the first range to start on a fresh range later
      ++cur;
      ++row;
      while (cur <= max) {
        assert(a.view().in(cur));
        add_to_mask(a.supports[row],mask);
        ++cur;
        ++row;
      }      
      ++rngs;
    
      // New, fresh range
      while (rngs()) {
        cur = rngs.min();
        max = rngs.max();
        row = a.supports.row(cur);
      
        while (cur <= max) {
          assert(a.view().in(cur));
          add_to_mask(a.supports[row],mask);
          ++cur;
          ++row;
        }
        ++rngs;
      }
#endif // CLEAR_MASK
    
      // Int::ViewRanges<View> rngs(a.view());
      intersect_with_mask(mask);
      break;
    }

    assert(nzerowords());
  }
  
  forceinline virtual ExecStatus
  advise(Space& home, Advisor& a0, const Delta& d) {
    CTAdvisor<View>& a = static_cast<CTAdvisor<View>&>(a0);
    View x = a.view();
      
    // Do not fail a disabled propagator
    if (is_empty())
      return disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;

    assert(limit >= 0);
    assert(nzerowords());
        
    // Do not schedule if propagator is performing propagation,
    // and dispose if assigned
    if (status == PROPAGATING) {
      if (x.assigned())
        return home.ES_FIX_DISPOSE(c,a);
      return ES_FIX;
    }

    ModEvent me = View::modevent(d);
    if (me == ME_INT_VAL) { // Variable is assigned -- intersect with its value
      int row = a.supports.row(x.val());
      intersect_with_mask_sparse_one(a.supports[row]);
    }
#ifdef DELTA
     else if (x.any(d)){ // No delta information -- do incremental update
      reset_based_update(a,home);
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
         for (int i = min_row; i <= max_row; i+=2) { // Nand supports two and two
           if (i != max_row) {                 // At least two values left
             assert(i + 1 <= max_row);
             const BitSet& s1 = a.supports[i];
             const BitSet& s2 = a.supports[i+1];
             if (!s1.empty() && !s2.empty()) { // Both non-empty
               nand_with_mask_two(s1,s2);
             } else if (!s1.empty()) {         // s1 non-empty, s2 empty
               nand_with_mask_one(s1);
             } else if (!s2.empty()) {         // s2 non-empty, s1 empty
               nand_with_mask_one(s2);
             }
           } else {                            // Last value
             assert(static_cast<unsigned int>(max_row - min_row + 1) % 2 == 1);
             const BitSet& s = a.supports[i];
             if (!s.empty()) 
               nand_with_mask_one(s);
           }
         }
      
       } else { // Domain size smaller than delta, reset-based update
         reset_based_update(a,home);
       }
     } 
#else
    else {
      reset_based_update(a,home);
    }
#endif // DELTA

    // Do not fail a disabled propagator
    if (is_empty())
      return disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;
    
    assert(nset() > 0);
    assert(limit >= 0);
    assert(nzerowords());
    
    // Update touched_var
    if (touched_var == -1) // no touched variable yet!
      touched_var = a.index;
    else if (touched_var != a.index) // some other variable is touched
      touched_var = -2;

    // Schedule propagator and dispose if assigned
    if (a.view().assigned()) {
      unassigned--;
      return home.ES_NOFIX_DISPOSE(c,a);
    }
    return ES_NOFIX;
  }
  
  forceinline ExecStatus
  filterDomains(Space& home) {
    if (unassigned == 0)
      return home.ES_SUBSUMED(*this);

    assert(limit >= 0);
    // Count the number of scanned unassigned variables
    unsigned int count_unassigned = unassigned;
    // Array to collect values to remove
    Region r;
    int* nq = r.alloc<int>(max_dom_size);
    int* nq_start = nq; 
    
    // Scan all values of all unassigned variables to see if they
    // are still supported.
    for (Advisors<CTAdvisor<View> > a0(c);
         a0() && count_unassigned; // End if only assigned variables left
         ++a0) {
      CTAdvisor<View>& a = a0.advisor();
      View v = a.view();
      int i = a.index;
      
      // No point filtering variable if it was the only modified variable
      if (touched_var == i) {
        continue;
      }

      switch (v.size()) {
      case 1: {  // Variable assigned, nothing to be done.
        break;
      }
      case 2: { // Consider min and max values
        const int min_val = v.min();
        const int max_val = v.max();
        const unsigned int offset = a.offset;

        // Fix to max_val if min_val not supported
        const int row_min = a.supports.row(min_val);
        if (!supported(a,row_min,offset)) {
          GECODE_ME_CHECK(v.eq(home,max_val));          
          --unassigned;
          break;
        }

        // Fix to min_val if max_val not supported
        const int row_max = a.supports.row(max_val);
        if (!supported(a,row_max,offset)) {
          GECODE_ME_CHECK(v.eq(home,min_val));    
          --unassigned;
          break;
        }

        // Otherwise v is still unassigned
        count_unassigned--;
        break;
      } default:
#ifdef LONG_FILTER
        const unsigned int offset = a.offset;
        const int min_val = v.min();
        const int max_val = v.max();
        int new_min = min_val;
        int new_max = max_val;
        int nremoves = 0;
        int last_support;
        
        // Iterate over single range if domain is an interval
        if (v.range()) {
          // Increase new_min to smallest supported value
          int row = a.supports.row(new_min);
          for (; new_min <= max_val; ++new_min) {
            if (supported(a,row,offset))
              break;
            ++row;
          }
          ModEvent me_gq = v.gq(home,new_min);
          if (me_failed(me_gq))
            return ES_FAILED;
    
          if (me_gq == ME_INT_VAL) {
            --unassigned;
            break;
          }
          // Decrease new_max to largest supported value
          row = a.supports.row(new_max);
          for (; new_max >= new_min; --new_max) {
            if (supported(a,row,offset))
              break;
            --row;
          }
          ModEvent me_lq = v.lq(home,new_max);
          if (me_failed(me_lq)) 
            return ES_FAILED;
     
          if (me_lq == ME_INT_VAL) {
            --unassigned;
            break;
          }
          // Filter out values in between min and max
          row = a.supports.row(new_min + 1);
          for (int val = new_min + 1; val < new_max; ++val) {
            if (!supported(a,row,offset))
              nq[nremoves++] = val;
            else
              last_support = val;
            ++row;
          }
          
        } else { // Domain is not a range
          new_min = Limits::max; // Escape value

          Int::ViewRanges<View> rngs(v);
          int cur;
          int max;
          int row;
          while (rngs()) {
            cur = rngs.min();
            max = rngs.max();
            row = a.supports.row(cur);
            while (cur <= max) {
              assert(a.view().in(cur));
              if (!supported(a,row,offset)) {
                nq[nremoves++] = cur;
              } else {
                if (new_min == Limits::max) {
                  new_min = cur;
                  nremoves = 0; // Will be covered by gq
                }
                new_max = cur; // Will collect the largest supported value
                last_support = cur;
              }
              ++cur;
              ++row;
            }
            ++rngs;
          }

          // Perform bounds propagation first
          ModEvent me_gq = v.gq(home,new_min);
          if (me_failed(me_gq))
            return ES_FAILED;
          
          if (me_gq == ME_INT_VAL) {
            --unassigned;
            break;
          }
          ModEvent me_lq = v.lq(home,new_max);
          if (me_failed(me_lq))
            return ES_FAILED;
          if (me_lq == ME_INT_VAL) {
            --unassigned;
            break;
          }
          // Trim nremoves
          while (nremoves > 0 && nq[nremoves-1] > new_max)
            nremoves--;
        }
#else
        unsigned int offset = a.offset;
        Int::ViewRanges<View> rngs(v);
        int cur, max, row;
        int last_support;
        nq = nq_start;
        while (rngs()) {
          cur = rngs.min();
          max = rngs.max();
          row = a.supports.row(cur);
          while (cur <= max) {
            assert(v.in(cur));
            if (!supported(a,row,offset))
              *(nq++) = cur;
            else
              last_support = cur;
            ++cur;
            ++row;
          }
          ++rngs;
        }
        unsigned int nremoves = static_cast<unsigned int>(nq - nq_start);
        
#endif // LONG_FILTER
        
        // Remove collected values
        if (nremoves > 0) {
          assert(nremoves < v.size());
          if (nremoves == v.size() - 1) {
            GECODE_ME_CHECK(v.eq(home,last_support));
       
            --unassigned;
            break;
          } else {
            Iter::Values::Array r(nq_start,nremoves);
            ModEvent me = v.minus_v(home,r,false);
            if (me_failed(me))
              return ES_FAILED;
            if (me == ME_INT_VAL) {
              --unassigned;
              break;
            }
          }
        }
        --count_unassigned;
      }
    }

    assert(unassignedCorrect());

    // Subsume if there is at most one non-assigned variable
    return unassigned <= 1 ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  forceinline bool
  supported(CTAdvisor<View>& a, int row, unsigned int offset) {
    int r = static_cast<int>(a.residues[row - offset]);
    const BitSet& support_row = a.supports[row];

    if (r == 0)
      return !BitSet::a(words,r,support_row,index[r]).none();

    if (r > limit)
      r = intersect_index(support_row, limit);
    else if (!BitSet::a(words,r,support_row,index[r]).none())
      return true;
    else
      r = intersect_index(support_row, r-1);

    if (r != -1) {
      assert(r >= 0);
      a.residues[row - offset] = static_cast<unsigned int>(r);
      return true;
    } 
    return false;
  }

#ifdef FIX
  forceinline ExecStatus
  fixDomains(Space& home) {
    // Only one valid tuple left, so we can fix all vars to that tuple
    unsigned int tuple_index = index_of_fixed();
    TupleSet::Tuple t = tupleSet[tuple_index];
    for (Advisors<CTAdvisor<View> > a0(c); a0(); ++a0) {
      CTAdvisor<View>& a = a0.advisor();
      View v = a.view();
      if (!v.assigned())
        GECODE_ME_CHECK(v.eq(home,t[a.index]));
    }
    return home.ES_SUBSUMED(*this);
  }
#endif // FIX
  
  // Dispose propagator and return its size
  forceinline virtual size_t
  dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    c.dispose(home);
#ifdef FIX
    (void) tupleSet.~TupleSet();    
#endif // FIX
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /** Debugging **/
  bool nzerowords() const {
    for (int i = 0; i <= limit; i++) {
      if (words.getword(i).none()) {
        printf("word %d is zero\n", i);
        printf("limit=%d\n", limit);
        
        return false;
      }
    }
    return true;
  }
  
  bool unassignedCorrect() {
    int count_unassigned = 0;
    for (Advisors<CTAdvisor<View> > a0(c); a0(); ++a0) {
      CTAdvisor<View>& a = a0.advisor();
      count_unassigned += !a.view().assigned();
    }

    return count_unassigned == unassigned;
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
