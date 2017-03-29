#include <gecode/int.hh>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
//#include "bitset.hpp"
#include "bitset-support.cpp"
//#include <unordered_map>

#define DEBUG 1

#define SHARED 1

using namespace Gecode;
using namespace Gecode::Int;
using namespace std;

// The compact table propagator
template<class View>
class CompactTable : public Propagator {
protected:
  // The variables
  ViewArray<View> x;
  // The table with possible combinations of values
  SparseBitSet<Space&> validTuples;
  // Starting idx for variable i in supports
  unsigned int* start_idx;
  // Smallest initial value for variable i
  int* start_val;
  // Residues
  unsigned int* residues;
  // Size of variable since last propagation
  unsigned int* lastSize;
  // Sum of domain sizes
  int domsum;
  // Nr of inital supports
  int nsupports;
#ifdef SHARED
  SharedSupports s;
#else
  // Supported tuples
  BitSet* supports;
#endif // SHARED

public:
  // Post table propagator
  static ExecStatus post(Home home,
                         ViewArray<View>& x,
                         TupleSet t) {
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
  CompactTable(Home home,
               ViewArray<View>& x0,
               TupleSet t0)
    : Propagator(home), x(x0),
      validTuples(static_cast<Space&>(home))
  {
    // Subscribe variables
    x.subscribe(home,*this,PC_INT_DOM);
    
    // Calculate domain sum
    domsum = 0;
    for (int i = 0; i < x.size(); i++) {
      domsum += x[i].width();
    }
    // Allocate memory
#ifdef SHARED
#else
    supports = static_cast<Space&>(home).alloc<BitSet>(domsum);    
#endif // SHARED

    start_idx = static_cast<Space&>(home).alloc<unsigned int>(x.size());
    start_val = static_cast<Space&>(home).alloc<int>(x.size());
    lastSize = static_cast<Space&>(home).alloc<unsigned int>(x.size());
    residues = static_cast<Space&>(home).alloc<unsigned int>(domsum);
    // Initialise start_val, start_idx
    for (int i = 0; i < x.size(); i++) {
      start_val[i] = x[i].min();
      if (i == 0)
        start_idx[i] = 0;
      else
        start_idx[i] = start_idx[i-1] + x[i-1].width();
    }
    // Initialise supports
    
    nsupports = init_supports(home, t0);
    // Initialise validTupels with nsupports bit set
    if (nsupports <= 0) {
      home.fail();
      return;
    }
    // Set the domain sizes in lastSize
    for (int i = 0; i < x.size(); i++)
      lastSize[i] = x[i].size();

    validTuples.init(t0.tuples(), nsupports);
  }
  
  unsigned int init_supports(Home home, TupleSet ts) {
    //cout << "in init_support" << endl;
#ifdef SHARED
    s.init_supports(domsum,x.size(),ts.tuples());
    for (int i = 0; i < x.size(); i++) {
      s.set_start_idx(i,start_idx[i]);
      s.set_start_val(i,start_val[i]);
    }
#else    
    // Initialise supports
    for (int i = 0; i < domsum; i++) {
      supports[i].init(static_cast<Space&>(home), ts.tuples(), false);
    }
#endif // SHARED
    
    
    int support_cnt = 0;
#ifdef SHARED
    int bpb = 64;
#else    
    int bpb = supports[0].get_bpb();//Gecode::Support::BitSetData::data(1);
#endif // SUPPORT

    // Look for supports and set correct bits
    for (int i = 0; i < ts.tuples(); i++) {
      bool supported = true;
      bool seen = true;
      for (int j = ts.arity() - 1; j >= 0; j--) {
        if (!x[j].in(ts[i][j])) {
          supported = false;
          break;
        } else if (support_cnt > 0) {
          // To filter out copies
#ifdef SHARED
          seen = seen && s.get(j,ts[i][j]).get(support_cnt - 1);          
#else          
          seen = seen && supports[rowno(j,ts[i][j])].get(support_cnt - 1);
#endif // SHARED
        }
      }
      if (supported && (support_cnt == 0 || !seen)) {
        // Set tuple as valid and save residue
        for (int j = 0; j < ts.arity(); j++) {
          int row = rowno(j, ts[i][j]);
#ifdef SHARED
          s.get(j,ts[i][j]).set(support_cnt);
#else
          supports[row].set(support_cnt);          
#endif // SHARED
          residues[row] = support_cnt / bpb;
        }
        support_cnt++;
      }
    }
    for (int i = 0; i < x.size(); i++) {
      Int::ViewValues<View> it(x[i]);
      vector<int> rvals; //values to remove
      while (it()) {
        // Remove value if row is empty
        // if (supports[rowno(i,it.val())].none()) {
        //   rvals.push_back(it.val());
        // }
#ifdef SHARED
        if (s.get(i,it.val()).none()) {
          rvals.push_back(it.val());
        }
#else        
        if (supports[rowno(i,it.val())].none()) {
          rvals.push_back(it.val());
        }
#endif // SHARED
        ++it;
      }
      Iter::Values::Array r(&rvals[0], rvals.size());
      if (::Gecode::me_failed(x[i].minus_v(home,r))) {
        return -1;
      }
    }
    return support_cnt;
  }

  unsigned int rowno(int var, int val) {
    return start_idx[var] + val - start_val[var];
  }
    
  // Copy constructor during cloning
  CompactTable(Space& home, bool share, CompactTable& p)
    : Propagator(home,share,p),
      validTuples(home, p.validTuples),
      domsum(p.domsum),
      nsupports(p.nsupports) {
    // Update view
    x.update(home,share,p.x);

    // Allocate memory
    start_val = home.alloc<int>(x.size());
    start_idx = home.alloc<unsigned int>(x.size());
    lastSize = home.alloc<unsigned int>(x.size());
#ifdef SHARED
    s.update(home,share,p.s);
#else    
    supports = home.alloc<BitSet>(domsum);    
#endif // MACRO
    residues = home.alloc<unsigned int>(domsum);
    for (int i = 0; i < x.size(); i++) {
      lastSize[i] = p.lastSize[i];
      // Don't bother to copy assigned variables
      if (x[i].size() > 1) {
        start_val[i] = p.start_val[i];
        start_idx[i] = p.start_idx[i];
        Int::ViewValues<View> it(x[i]);
        while (it()) {
          unsigned int row = p.rowno(i,it.val());
          residues[row] = p.residues[row];
#ifndef SHARED
          supports[row].init(home,nsupports,false);
          supports[row].copy(nsupports,p.supports[row]);
#endif // SHARED
          ++it;
        }
      }
    }
  }
  
  // Create copy during cloning
  virtual Propagator* copy(Space& home, bool share) {
    return new (home) CompactTable(home,share,*this);
  }
    
  // Return cost (defined as cheap quadratic)
  virtual PropCost cost(const Space&, const ModEventDelta&) const {
    // TODO: ???
    return PropCost::linear(PropCost::LO,2*x.size());
  }

  // TODO: ???
  virtual void reschedule(Space& home) {
    x.reschedule(home,*this,Int::PC_INT_DOM);
  }
  
  // Perform propagation
  virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
    updateTable();
    
    if (validTuples.is_empty()) {
      return ES_FAILED;
    }
    ExecStatus msg = filterDomains(home);
    return msg;
  }

  void updateTable() {
    //cout << "updateTable" << endl;
    for (int i = 0; i < x.size(); i++) {
      if (lastSize[i] == x[i].size()) {
        continue;
      }
      lastSize[i] = x[i].size();
      validTuples.clear_mask();
      Int::ViewValues<View> it(x[i]);
      while (it()) {
        //  cout << it.val() << endl;
#ifdef SHARED
        validTuples.add_to_mask(s.get(i,it.val()));        
#else
        validTuples.add_to_mask(supports[rowno(i,it.val())]);        
#endif // SHARED
        ++it;
      }
      validTuples.intersect_with_mask();
      if (validTuples.is_empty()) {
        return;
      }
    }
  }

  ExecStatus filterDomains(Space& home) {
    int count_non_assigned = 0;
    for (int i = 0; i < x.size(); i++) {
      // only filter out values for variables with domain size > 1
      if (x[i].size() > 1) {
        Int::ViewValues<View> it(x[i]);
        vector<int> rvals; //values to remove
        while (it()) {
          int index = residues[rowno(i,it.val())];
          int row = rowno(i,it.val());

#ifdef SHARED
          Support::BitSetData w = validTuples.a(s.get(i,it.val()),index);
#else          
          Support::BitSetData w = validTuples.a(supports[row],index);
#endif // SHARED
          
          if (w.none()) {
#ifdef SHARED
            index = validTuples.intersect_index(s.get(i,it.val()));
#else            
            index = validTuples.intersect_index(supports[row]);
#endif // SHARED
            if (index != -1) {
              // Save residue
              residues[rowno(i,it.val())] = index;
            } else {
              // Value not supported
              rvals.push_back(it.val());
            }
          }
          ++it;
        }
        Iter::Values::Array r(&rvals[0], rvals.size());
        GECODE_ME_CHECK(x[i].minus_v(home,r));
        if (x[i].size() > 1) {
          ++count_non_assigned;
        }
        lastSize[i] = x[i].size();
      }
    }
    // Subsume if there is at most one non-assigned variable
    return count_non_assigned <= 1 ? home.ES_SUBSUMED(*this) : ES_FIX;
  }
  
  // Dispose propagator and return its size
  virtual size_t dispose(Space& home) {
    x.cancel(home,*this,PC_INT_DOM);
    // TODO: dispose t?
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
    cout << "lastSize: ";
    for (int i = 0; i < x.size(); i++) {
      cout << lastSize[i] << " ";
    }
    cout << endl;
    cout << "startVal: ";
    for (int i = 0; i < x.size(); i++) {
      cout << start_val[i] << " ";
    }
    cout << endl;
    cout << "startIdx: ";
    for (int i = 0; i < x.size(); i++) {
      cout << start_idx[i] << " ";
    }
    cout << endl;
    
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
  //   sprintf(buf, "%d%d", var, val);
  //   mapkey key(buf);
  //   return key;
  // }
};


// Post the table constraint
namespace Gecode {
  
  void
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

  void extensional2(Home home, const BoolVarArgs& x, const TupleSet& t) {
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
