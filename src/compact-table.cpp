#include <gecode/int.hh>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
//#include "bitset.hpp"
#include "bitset-support.cpp"
//#include <unordered_map>

//#define DEBUG 1

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
  /// Index of the view
  int index;
  /// Constructor
  forceinline
  CTAdvisor(Space& home, Propagator& p,
            Council<CTAdvisor<View> >& c,
            View x0, int i)
    : ViewAdvisor<View>(home,p,c,x0), index(i) {}

  /// Copy constructor
  forceinline
  CTAdvisor(Space& home, bool share, CTAdvisor<View>& a)
    : ViewAdvisor<View>(home,share,a),
    index(a.index) {}

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
  enum Status {NOT_PROPAGATE,PROPAGATE};
  /// Whether propagator is done propagating
  Status status;
  /// The variables
  ViewArray<View> x;
  /// Council of advisors
  Council<CTAdvisor<View> > c;
  // The table with possible combinations of values
  SparseBitSet<Space&> validTuples;
#ifdef HASH
  /// Residues
  HashTable residues;
#else
  /// Residues
  unsigned int* residues;
#endif // HASH
  // Size of variable since last propagation
  unsigned int* lastSize;
  // Sum of domain sizes
  int domsum;
  // Nr of inital supports
  int nsupports;
  // Supports
  SharedSupports s;
  /// Number of unassigned variables
  unsigned int unassigned;
  
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
      status(NOT_PROPAGATE)
  {
    DEBUG_PRINT(("Start constructor\n"));
    // Calculate domain sum
    domsum = 0;
    int domsize = 0;
    for (int i = 0; i < x.size(); i++) {
      domsum += x[i].width();
      domsize += x[i].size();
    }
    DEBUG_PRINT(("domsum: %d, domsize: %d\n", domsum, domsize));

    // Allocate memory
    s.init_supports(domsum,x.size(),t0.tuples());
#ifndef HASH
    for (int i = 0; i < x.size(); i++) {
      s.set_start_val(i,x[i].min());
      if (i == 0)
        s.set_start_idx(i,0);
      else
        s.set_start_idx(i,s.get_start_idx(i-1) + x[i-1].width());
    }
#endif // not HASH
    lastSize = static_cast<Space&>(home).alloc<unsigned int>(x.size());
#ifdef HASH
    residues.init(HashTable::closest_prime(1.7*domsize));
#else    
    residues = static_cast<Space&>(home).alloc<unsigned int>(domsum);
#endif // HASH
    
    // Initialise supports
    nsupports = init_supports(home, t0);
    DEBUG_PRINT(("nsupports=%d\n",nsupports));
    
    if (nsupports <= 0) {
      home.fail();
      return;
    } 
    // Set the domain sizes in lastSize
    for (int i = 0; i < x.size(); i++)
      lastSize[i] = x[i].size();
    
    // Initialise validTupels with nsupports bit set
    validTuples.init(t0.tuples(), nsupports);
    DEBUG_PRINT(("End constructor\n"));

    // Post advisors
    unassigned = x.size();
    for (int i = x.size(); i--; ) {
      if (!x[i].assigned()) {
        (void) new (home) CTAdvisor<View>(home,*this,c,x[i],i);
      } else {
        unassigned--;
      }
    }

    DEBUG_PRINT(("nvars: %d, unassigned: %d\n",x.size(),unassigned));
    
    // TODO
    View::schedule(home,*this,Int::ME_INT_VAL);
  }

  forceinline unsigned int
  init_supports(Home home, TupleSet ts) {
    Region region(home);
    // Bitset for O(1) access to domains
    Dom dom = region.alloc<BitSet>(x.size());
    init_dom(home,dom,ts.min(),ts.max());
#ifdef HASH
    s.fill(dom,x.size(),ts.min(),ts.max());    
#endif // HASH
    int support_cnt = 0;
    int bpb = BitSet::get_bpb();

    // Look for supports and set correct bits
    for (int i = 0; i < ts.tuples(); i++) {
      bool supported = true;
      bool seen = true;
      for (int j = ts.arity() - 1; j >= 0; j--) {
        if (!dom[j].get(ts[i][j] - ts.min())) {
          supported = false;
          break;
        } else if (support_cnt > 0) {
          // To filter out copies
          seen = seen && s.get(j,ts[i][j]).get(support_cnt - 1);
        }
      }
      if (supported && (support_cnt == 0 || !seen)) {
        // Set tuple as valid and save residue
        for (int j = 0; j < ts.arity(); j++) {
          unsigned int row = s.row(j,ts[i][j]);
          s.get(j,ts[i][j]).set(support_cnt);
#ifdef HASH
          Key key = {j,ts[i][j]};
          Item* item = heap.alloc<Item>(1);
          item->key = key; item->row = support_cnt / bpb;
          residues.insert(item);
#else          
          residues[row] = support_cnt / bpb;
#endif // HASH
        }
        support_cnt++;
      }
    }

    Region r(home);
    Support::StaticStack<int,Region> nq(r,domsum);
        
    for (int i = 0; i < x.size(); i++) {
      Int::ViewValues<View> it(x[i]);
      while (it()) {
        if (s.get(i,it.val()).none()) {
          nq.push(it.val());
        }
        ++it;
      }

      while (!nq.empty()) {
        GECODE_ME_CHECK(x[i].nq(home,nq.pop()));
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
      nsupports(p.nsupports),
      status(p.status)
#ifdef HASH
    , residues(p.residues)
#endif // HASH
  {
    DEBUG_PRINT(("Copy constructor\n"));
    x.update(home,share,p.x);
    s.update(home,share,p.s);
    c.update(home,share,p.c);

    DEBUG_PRINT(("Done updating datastructures\n"));
    // Allocate memory
    lastSize = home.alloc<unsigned int>(x.size());
#ifndef HASH
    residues = home.alloc<unsigned int>(domsum);
#endif // HASH
    DEBUG_PRINT(("Done allocating memory\n"));
    
    for (int i = 0; i < x.size(); i++) {
      lastSize[i] = p.lastSize[i];
      // Don't bother to copy assigned variables
#ifndef HASH
      if (x[i].size() > 1) {
        Int::ViewValues<View> it(x[i]);
        while (it()) {
          unsigned int row = s.row(i,it.val());
          residues[row] = p.residues[row];
          ++it;
        }
      }
#endif // HASH
    }
    DEBUG_PRINT(("End copy constructor\n"));
  }
  
  // Create copy during cloning
  forceinline virtual Propagator*
  copy(Space& home, bool share) {
    return new (home) CompactTable(home,share,*this);
  }
    
  // Return cost (defined as cheap quadratic)
  forceinline virtual PropCost
  cost(const Space&, const ModEventDelta&) const {
    // TODO: ???
    return PropCost::linear(PropCost::LO,2*x.size());
  }

  // TODO: ???
  forceinline virtual void
  reschedule(Space& home) {
    x.reschedule(home,*this,Int::PC_INT_DOM);
  }
  
  // Perform propagation
  forceinline virtual ExecStatus
  propagate(Space& home, const ModEventDelta&) {
    DEBUG_PRINT(("Propagate\n"));
    status = PROPAGATE;
    //updateTable();
    
    if (validTuples.is_empty()) {
      return ES_FAILED;
    }
    ExecStatus msg = filterDomains(home);
    DEBUG_PRINT(("End propagate\n"));
    status = NOT_PROPAGATE;
    return msg;
  }

  forceinline void
  updateTable() {
    DEBUG_PRINT(("updateTable"));
    for (int i = 0; i < x.size(); i++) {
      if (lastSize[i] == x[i].size()) {
        continue;
      }
      updateTable(i);
      if (validTuples.is_empty()) {
        return;
      }
    }
    DEBUG_PRINT(("End updateTable\n"));
  }

  forceinline void
  updateTable(int i) {
    DEBUG_PRINT(("Update table %d\n", i));
    lastSize[i] = x[i].size();
    validTuples.clear_mask();
    Int::ViewValues<View> it(x[i]);
    while (it()) {
      validTuples.add_to_mask(s.get(i,it.val()));        
      ++it;
    }
    validTuples.intersect_with_mask();
  }

  forceinline virtual ExecStatus
  advise(Space& home, Advisor& a0, const Delta& d) {
    CTAdvisor<View> a = static_cast<CTAdvisor<View>&>(a0);
    DEBUG_PRINT(("Advise %d\n", a.index));
    DEBUG_PRINT(("Modevent: %d\n",a.view().modevent(d)));
    if (status == PROPAGATE) {
      // Do not schedule if propagator is performing propagation
      return ES_FIX;
    }
    updateTable(a.index);
    if (validTuples.is_empty())
      // Not allowed to report failure in a disabled propagator
      return disabled() ? home.ES_FIX_DISPOSE(c,a) : ES_FAILED;
    if (a.view().assigned()) {
      --unassigned;
      DEBUG_PRINT(("Variable %d assigned in advise\n",a.index));
      //      return unassigned <= 1 ? home.ES_SUBSUMED(*this) : home.ES_NOFIX_DISPOSE(c,a);
      return  home.ES_NOFIX_DISPOSE(c,a);
    }
    return ES_NOFIX;
  }
      
  
  forceinline ExecStatus
  filterDomains(Space& home) {
    int count_non_assigned = 0;
    Region r(home);
    Support::StaticStack<int,Region> nq(r,domsum);
    // Only consider unassigned variables
    for (Advisors<CTAdvisor<View> > a(c); a(); ++a) {
      int i = a.advisor().index;
      if (a.advisor().view().assigned()) {
        DEBUG_PRINT(("Variable %d is assigned\n",i));
        continue;
      }

      DEBUG_PRINT(("Advisor for %d with domain size %d\n", i, x[i].size()));
      Int::ViewValues<View> it(x[i]);
      while (it()) {
#ifdef HASH
        Key key = {i,it.val()};
        int index = residues.get(key);
#else
        unsigned int row = s.row(i,it.val());
        int index = residues[row];          
#endif // HASH

        Support::BitSetData w = validTuples.a(s.get(i,it.val()),index);

        if (w.none()) {
          index = validTuples.intersect_index(s.get(i,it.val()));
          if (index != -1) {
            // Save residue
#ifdef HASH
            residues.set((Key){i,it.val()},index);
#else
            residues[row] = index;
#endif // HASH

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
        GECODE_ME_CHECK(x[i].nq(home,val));
      }
        
      if (!x[i].assigned()) {
        //        a.advisor().dispose(home,c);
        count_non_assigned++;
        //--unassigned;
      } else {
        DEBUG_PRINT(("Variable %d assigned in filterDomains\n",i));
        //a.advisor().dispose(home,c);
      }
      lastSize[i] = x[i].size();
    }
    // Subsume if there is at most one non-assigned variable
    return count_non_assigned <= 1 ? home.ES_SUBSUMED(*this) : ES_FIX;
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

private:

  void print_supports() {
    // int count = 0;
    // for (int i = 0; i < domsum; i++) {
    //   count += supports[i].nset();
    //   //supports[i].print();
    // }
    // cout << count << endl;
    // // for (int i = 0; i < x.size(); i++) {
    // //   //cout << "domain for variable " << i << ": " << x[i] << endl;
    // //   Int::ViewValues<View> it(x[i]);
    // //   while (it()) {
    // //     // if (i == 0 && it.val() == 1) {
    // //     //   cout << "(0,1):" << rowno(i,it.val()) << endl;
    // //     //   cout << start_idx[0] << "+" << 1 << "-" << start_val[0];
    // //     //   cout << "=" << start_idx[0] + 1 - start_val[0] << endl;
    // //     // }
    // //     //cout << rowno(i,it.val()) << ": ";
    // //     supports[rowno(i,it.val())].print();
    // //     ++it;
    // //   }
    // // }
  }

  void print_stuff() {
    // cout << "lastSize: ";
    // for (int i = 0; i < x.size(); i++) {
    //   cout << lastSize[i] << " ";
    // }
    // cout << endl;
    // cout << "startVal: ";
    // for (int i = 0; i < x.size(); i++) {
    //   cout << start_val[i] << " ";
    // }
    // cout << endl;
    // cout << "startIdx: ";
    // for (int i = 0; i < x.size(); i++) {
    //   cout << start_idx[i] << " ";
    // }
    // cout << endl;
    
  }
  
  
  // void fill_row_map() {
  //   int row_cnt = 0;
  //   for (int j = 0; j < x.size(); j++) {
  //     Int::ViewValues<View> i(x[j]);
  //     while (i()) {
  //       rmap_entry entry(key_of(j, i.val()), row_cnt);
  //       row_map.insert(entry);
  //       ++i; ++row_cnt;
  //     }
  //   }
  // }

  // void print_row_map() {
  //   for (int j = 0; j < x.size(); j++) {
  //     Int::ViewValues<View> i(x[j]);
  //     while (i()) {
  //       cout << "(" << j << ", " << i.val() << ") with key value: " <<
  //         key_of(j, i.val()) <<
  //         " is at row " <<
  //         row_map.at(key_of(j, i.val())) << endl;
  //       ++i;
  //     }
  //   }
  // }

  // mapkey key_of(int var, int val) {
  //   char buf[2048];
  //   sDEBUG_PRINT(buf, "%d%d", var, val);
  //   mapkey key(buf);
  //   return key;
  // }
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
