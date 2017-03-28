#include <gecode/int.hh>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
#include "bitset.hpp"
//#include <unordered_map>

//#define DEBUG 1

int nprop = 0;

using namespace Gecode;
using namespace Gecode::Int;
using namespace std;

//typedef Support::BitSetBase BitSet;

typedef uint64_t word_t;
//typedef string mapkey;
//typedef unordered_map<mapkey, int> rmap;
//typedef pair<mapkey, int> rmap_entry;

#ifdef GECODE_SUPPORT_MSVC_64
    /// Basetype for bits
    typedef unsigned __int64 Base;
#else
    /// Basetype for bits
    typedef unsigned long int Base;
#endif

static const unsigned int bpb =
  static_cast<unsigned int>(CHAR_BIT * sizeof(Base));

#define BITS_PER_WORD bpb

// Class for maintaining the supported tuples
class SparseBitSet {
  vector<word_t> words;
  vector<word_t> mask;

  vector<int> index; // type?
  int limit;
  unsigned int nbits;

public:
  SparseBitSet(Home home, unsigned int _nbits) {
    nbits = _nbits;
    unsigned int nwords = required_words(nbits);
    
    // Initialise words, mask and index
    for (int i = 0; i < nwords; i++) {
      words.push_back(~0ULL); // Set all bits to 1
      mask.push_back(0ULL);
      index.push_back(i);
    }

    /// Limit is initially highest index
    limit = nwords - 1;
    //words_bs->data;

  }

  /// Copy constructor
  SparseBitSet(Home home, SparseBitSet& sbs) :
    words(sbs.words),
    mask(sbs.mask),
    index(sbs.index),
    limit(sbs.limit),
    nbits(sbs.nbits) {}
  
  /// Return true if bitset is empty, else false
  bool is_empty() const {
    return limit == -1;
  }
  
  /// Clear all bits in mask
  void clear_mask() {
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      //mask_bs[offset].clearall();
      mask.at(offset) = 0ULL;
    }
  }

  // /// Reverse bits in mask
  // void reverse_mask() {
  //   for (int i = 0; i <= limit; i++) {
  //     int offset = index[i];
  //     mask.at(offset) = ~mask.at(offset);
  //   }
  // }

  /// Add bits to mask
  void add_to_mask(vector<word_t> m) {
    assert(m.size() == mask.size());
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      
      mask.at(offset) |= m.at(offset);
    }
  }

  /// Intersect words with mask
  void intersect_with_mask() {
    for (int i = limit; i >= 0; i--) {
      int offset = index.at(i);
      word_t w = (words.at(offset) & mask.at(offset));
      if (w != words.at(offset)) {
        words.at(offset) = w;
        if (w == 0ULL) {
          index.at(i) = index.at(limit);
          index.at(limit) = offset;
          --limit;
        }
      }
    }
  }

  /* Returns the index of a word where the bit-set
  intersects with m, -1 otherwise */
  int intersect_index(vector<word_t> m) const {
    for (int i = 0; i <= limit; i++) {
      int offset = index.at(i);
      if ((words.at(offset) & m.at(offset)) != 0ULL) {
        return offset;
      }
    }
    return -1;
  }

  int nset() {
    int count = 0;
    for (int i = 0; i < required_words(nbits); i++) {
      for (int j = 0; j < BITS_PER_WORD; j++) {
        if((words.at(i) >> j) & 1ULL) {
          count++;
        }
      }
    }
    return count; 
  }
  
  /// Print bit-set for simple debugging
  void print(int ncols) {
    int count = 0;
    cout << "words: ";
    for (int i = 0; i < required_words(nbits); i++) {
      //cout << words.at(i) << endl;
      for (int j = 0; j < BITS_PER_WORD; j++) {
        word_t b = (words.at(i) >> j) & 1ULL;
        cout << b << " ";
        count++;
        if(count == ncols) {
          cout << endl;
          j = BITS_PER_WORD;
          i = required_words(nbits);
        }
      }
    }

    cout << "mask: ";
    for (int i = 0; i < required_words(nbits); i++) {
      //cout << words.at(i) << endl;
      for (int j = 0; j < BITS_PER_WORD; j++) {
        word_t b = (mask.at(i) >> j) & 1ULL;
        cout << b << " ";
        count++;
        if(count == ncols) {
          cout << endl;
          j = BITS_PER_WORD;
          i = required_words(nbits);
        }
      }
    }

    
    //cout << endl;

    // cout << "mask: ";
    // for (int i = 0; i < required_words(nbits); i++) {
    //   //cout << mask.at(i) << endl;
    //   for (int j = 0; j < BITS_PER_WORD; j++) {
    //     word_t b = (mask.at(i) >> j) & 1ULL;
    //     cout << b << " ";
    //   }
    //   cout << endl;
    // }
   
    cout << "index: ";
    for (int i = 0; i < required_words(nbits); i++) {
      cout << index.at(i) << " ";
    }
    cout << endl;
    cout << "limit: ";
    cout << limit << endl;;

    
  }

  // Return the number of ints needed for nbits bits
  unsigned int required_words(unsigned int nbits) {
    if (nbits == 0) {
      return 0;
    } else {
      return (nbits - 1) / BITS_PER_WORD + 1;
    }
  }
};

struct MyBitSet {
#define Toggle(a, i) (a) |= (1UL << (i))
#define Clear(a, i) (a) &= ~(1UL << (i))
#define Get(a, i) (((a) >> (i)) & 1UL)
  
  vector<word_t> atoms;
  size_t size;
  size_t atoms_per_row;

  MyBitSet(int rows, int cols)
    : atoms_per_row(required_ints_per_row(cols)) {
    size = required_ints(rows);
    for (int i = 0; i < size; i++) {
      atoms.push_back(0UL);
    }
  }

  size_t required_ints_per_row(int cols) {
    return ceil((double)cols/BITS_PER_WORD);
  }
  
  MyBitSet(const MyBitSet& other)
    : atoms(other.atoms)
    , size(other.size)
    , atoms_per_row(other.atoms_per_row) {}

  size_t required_ints(int rows) const {
    return atoms_per_row*rows;
  }

  void set(int r, int c) {
    Toggle(atoms.at(atom_idx(r,c)), bit_idx(c));
  }

  bool get(int r, int c) const {
    return Get(atoms.at(atom_idx(r,c)), bit_idx(c)) == 1UL;
  }

  vector<word_t> get_row(int r) {
    vector<word_t> row;
    for (int i = 0; i < atoms_per_row; i++) {
      row.push_back(atoms.at(atom_idx(r,i*BITS_PER_WORD)));
    }
    return row; 
  }
  
  void print(int ncols,ostream& os=cout) const {
    //cout << "size: " << size << endl;
    // cout << "atoms_per_row: " << atoms_per_row << endl;
    // cout << "ncols: " << ncols << endl;
    int count=0;
    for (int i = 0; i < size/atoms_per_row; i++) {
      for (int j = 0; j < atoms_per_row; j++) {
        for (int k = 0; k < BITS_PER_WORD; k++) {
          //os << get(i, j*BITS_PER_WORD + k) << " ";
          if (get(i, j*BITS_PER_WORD + k))
            count++;
          // if (count == ncols) {
          //   j = atoms_per_row;
          //   k = BITS_PER_WORD;
          //   //break;
          // }
        }
        //os << endl;
      }
    }
    os << count << endl;
  }
  
private:
  int atom_idx(int r, int c) const {
    return r*atoms_per_row + c/BITS_PER_WORD;
  }

  int bit_idx(int c) const {
    return c % BITS_PER_WORD;
  }
};

// The compact table propagator
class CompactTable : public Propagator {
protected:  
  // The variables
  ViewArray<IntView> x;
  // The table with possible combinations of values
  SparseBitSet currTable;
  // Supported tuples (static)
  MyBitSet supports;

  int* start_idx;
  int* start_val;
  
  // Row map for support entries
  //rmap row_map;
  // Last sizes
  vector<int> lastSize;

  // only for printing words
  int ntuples;
  
  int prop;
  // Refactored version
  NewSparseBitSet<Space&> validTuples;
  BitSet* newsupports;

  int nsupports;
  int domsum;
  
public:
  // Create propagator and initialize
  CompactTable(Home home,
               ViewArray<IntView>& x0, 
               TupleSet t0,
               int domsum0)
    : Propagator(home), x(x0), currTable(home, t0.tuples()),
      supports(domsum0, t0.tuples()),
      domsum(domsum0),
      validTuples(static_cast<Space&>(home))
  {
    newsupports = static_cast<Space&>(home).alloc<BitSet>(domsum);
    
    start_idx = static_cast<Space&>(home).alloc<int>(x.size());
    start_val = static_cast<Space&>(home).alloc<int>(x.size());
    //newsupports = static_cast<Space&>(home).alloc<BitSet>(domsum);

    ntuples = t0.tuples();
    prop = 0;
    
    int cnt = 0;
    for (int i = 0; i < x.size(); i++) {
      start_val[i] = x[i].min();
      start_idx[i] = cnt;
      cnt += x[i].width(); // ?
    }

    x.subscribe(home,*this,PC_INT_DOM);

    //fill_row_map();
    // Initalise supports
    int no_tuples = init_supports(home, t0);
    int no_tuples_2 = init_supports_2(home, t0);
    assert(no_tuples == no_tuples_2);
    nsupports = no_tuples;
    
    // Initialise lastSize with dummy value -1
    for (int i = 0; i < x.size(); i++) {
      lastSize.push_back(-1);
    }

    // Set no_tuples bits to 1 in currTable
    // FIXME: not nice
    MyBitSet bs(1, t0.tuples());
    for (int i = 0; i < no_tuples; i++) {
      bs.set(0, i);
    }
    currTable.add_to_mask(bs.get_row(0));
    currTable.intersect_with_mask();

    validTuples.init(t0.tuples());
    validTuples.clearall(nsupports, true);
    
#ifdef DEBUG
    cout << "Constuctor done \n Initial state:\n";
    cout << "Tuples: " << endl;
    for (int i = 0; i < t0.tuples(); i++) {
      for (int j = 0; j < t0.arity(); j++) {
        cout << t0[i][j] << " ";
      }
      cout << endl;
    }
    cout << "supports: " << endl;
    supports.print(no_tuples);
    cout << "currTable: " << endl;
    currTable.print(ntuples);    
#endif // DEBUG
    // cout << "supports: " << endl;
    // supports.print(no_tuples);
    // cout << "validTuples: " << endl;
    // currTable.print(ntuples);
    for (int i = 0; i < x.size(); i++) { 
      lastSize[i] = x[i].size();
    }

    assert_same_state();
  }

  void assert_same_state() {
    if (currTable.nset() != validTuples.nset()) {
      cout << "nset(currTable) = " << currTable.nset() << endl;
      currTable.print(ntuples);
      //currTable.print_mask(ntuples);
      cout << "nset(validTuples) = " << validTuples.nset() << endl;
      validTuples.print();
      //validTuples.print_mask();
    }
    assert(currTable.nset() == validTuples.nset());
    //assert(currTable.limit == validTuples.get_limit());
    //assert(false);
  }
  
  int rowno(int var, int val) {
    //int rowno1 = row_map.at(key_of(var, val));
    //int rowno2 = start_idx[var] + val - start_val[var];

    return start_idx[var] + val - start_val[var];
    //return row_map.at(key_of(var, val));
  }

  int init_supports(Home home, TupleSet ts) {
    int support_cnt = 0;
    for (int i = 0; i < ts.tuples(); i++) {
      bool supported = true;
      for (int j = 0; j < ts.arity(); j++) {
        if (!x[j].in(ts[i][j])) {
          supported = false;
          
#ifdef DEBUG
          cout << "Value " << ts[i][j] << " not supported for variable " << j << endl;
#endif // DEBUG
          
          break;
        }
      }
      if (supported) {
        // Set tuple as valid
        set_tuple(i, support_cnt, ts);
        support_cnt++;
      }
    }

    // Remove values corresponding to 0-rows
    for (int i = 0; i < x.size(); i++) {
      Int::ViewValues<Int::IntView> it(x[i]);
      vector<int> rvals; //values to remove
      while (it()) {
        vector<word_t> row = supports.get_row(rowno(i,it.val()));
        for (int j = 0; j < row.size(); j++) {
          if (row[j] != 0ULL) {
            break;
          }
          // The row is 0, remove that value
          if (j == row.size() - 1) {
            rvals.push_back(it.val());
          }
        }
        ++it;
      }
      Iter::Values::Array r(&rvals[0], rvals.size());
      GECODE_ME_CHECK(x[i].minus_v(home,r));
    }
   
    return support_cnt;
  }

  int init_supports_2(Home home, TupleSet ts) {
    for (int i = 0; i < domsum; i++) {
      newsupports[i].init(static_cast<Space&>(home), ts.tuples(), false);
    }

    int support_cnt = 0;
    int bpb = newsupports[0].get_bpb();
    
    for (int i = 0; i < ts.tuples(); i++) {
      bool supported = true;
      bool seen = true;
      for (int j = 0; j < ts.arity(); j++) {
        if (!x[j].in(ts[i][j])) {
          supported = false;
          break;
        } else if (support_cnt > 0) {
          // To filter out multiples
          seen = seen && newsupports[rowno(j,ts[i][j])].get(support_cnt - 1);
        }
      }
      if (supported && (support_cnt == 0 || !seen)) {
        // Set tuple as valid and save residue
        for (int j = 0; j < ts.arity(); j++) {
          int row = rowno(j, ts[i][j]);
          //          cout << "setting row " << row << ", corresponding to " << j << ", " << ts[i][j] << endl; 
          newsupports[row].set(support_cnt);
          //residues[row] = support_cnt / bpb;
        }
        support_cnt++;
      }
    }
    //print_supports();
    // Remove values corresponding to 0-rows
    for (int i = 0; i < x.size(); i++) {
      Int::ViewValues<Int::IntView> it(x[i]);
      vector<int> rvals; //values to remove
      while (it()) {
        //Gecode::Support::BitSetBase bs;
        //bs = newsupports[rowno(i,it.val())];
        if (newsupports[rowno(i,it.val())].none()) {
          rvals.push_back(it.val());
        }
        ++it;
      }
      Iter::Values::Array r(&rvals[0], rvals.size());
      GECODE_ME_CHECK(x[i].minus_v(home,r));
    }

    // Set the domain sizes in lastSize
    // for (int i = 0; i < x.size(); i++) { 
    //   lastSize[i] = x[i].size();
    // }
    
    return support_cnt;
  }


  void set_tuple(int t, int column, TupleSet ts) {
    assert(t <= ts.tuples());
    for (int i = 0; i < ts.arity(); i++) {
      supports.set(rowno(i, ts[t][i]), column);
    }
  }
  
  // Copy constructor during cloning
  CompactTable(Space& home, bool share, CompactTable& p)
    : Propagator(home,share,p),
      currTable(home, p.currTable),
      validTuples(home, p.validTuples),
      nsupports(p.nsupports),
      domsum(p.domsum),
      supports(p.supports),
      ntuples(p.ntuples),
      prop(p.prop),
      //row_map(p.row_map),
      lastSize(p.lastSize) {
#ifdef DEBUG
    cout << "copy constructor" << endl;
#endif // DEBUG
    x.update(home,share,p.x);
    start_val = home.alloc<int>(x.size());
    start_idx = home.alloc<int>(x.size());

    newsupports = home.alloc<BitSet>(domsum);
    
    for (int i = 0; i < x.size(); i++) {
      start_val[i] = p.start_val[i];
      start_idx[i] = p.start_idx[i];

      Int::ViewValues<Int::IntView> it(x[i]);
      while (it()) {
        int row = p.rowno(i,it.val());
        //residues[row] = p.residues[row];
        //newsupports[row] = BitSet(home, p.supports[row]);
        newsupports[row].init(home,nsupports,false);
        newsupports[row].copy(nsupports,p.newsupports[row]);//.copy(p.supports[row]);
        ++it;
      }
    }
    assert_same_state();
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
    if (x.size() > 1) {
      int domsum = 0;
      for (int i = 0; i < x.size(); i++) {
        //domsum += x[i].size();
        domsum += x[i].max() - x[i].min() + 1;
      }
      (void) new (home) CompactTable(home,x,t,domsum);
    }
    return ES_OK;
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

  virtual void reschedule(Space& home) {
    x.reschedule(home,*this,Int::PC_INT_DOM);
  }
  
  // Perform propagation
  virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
    // cout << "before propagation: " << prop << endl;
    // cout << "validTuples: " << endl;
    // currTable.print(ntuples);    
    assert_same_state();
    
    prop++;
    updateTable();
    if (currTable.is_empty()) {
      return ES_FAILED;
    }
    return filterDomains(home);

    assert_same_state();
    // cout << "after propagation: " << prop << endl;
    // cout << "validTuples: " << endl;
    // currTable.print(ntuples);    

  }

  void updateTable() {
    for (int i = 0; i < x.size(); i++) {
      if (lastSize.at(i) == x[i].size()) {
        continue;
      }
      lastSize.at(i) = x[i].size();
      
      currTable.clear_mask();
      validTuples.clear_mask();

      assert_same_state();
      
      Int::ViewValues<Int::IntView> it(x[i]);
      while (it()) {
        currTable.add_to_mask(supports.get_row(rowno(i,it.val())));
        validTuples.add_to_mask(newsupports[rowno(i,it.val())]);
        ++it;
      }

      currTable.intersect_with_mask();
      validTuples.intersect_with_mask();

      assert_same_state();
      
      if (currTable.is_empty() || validTuples.is_empty()) {
        if (!(currTable.is_empty() && validTuples.is_empty())) {
          cout << "currTable.is_empty = " << currTable.is_empty() << endl;
          currTable.print(nsupports);
          cout << "validTuples.is_empty = " << validTuples.is_empty() << endl;
          cout << "validTuples.none = " << validTuples.none() << endl;
          validTuples.print();
        }
        assert(currTable.is_empty() && validTuples.is_empty());
        return;
      }
    }
  }

  ExecStatus filterDomains(Space& home) {
    int count_non_assigned = 0;
    for (int i = 0; i < x.size(); i++) {
      // only filter out values for variables with domain size > 1
      if (x[i].size() > 1) {
        Int::ViewValues<Int::IntView> it(x[i]);
        vector<int> rvals; //values to remove
        while (it()) {
          int index = currTable.intersect_index(supports.get_row(rowno(i,it.val())));
          if (index != -1) {
            // save residue
          } else {
            rvals.push_back(it.val());
          }
          ++it;
        }
        Iter::Values::Array r(&rvals[0], rvals.size());
        GECODE_ME_CHECK(x[i].minus_v(home,r));
        if (x[i].size() > 1) {
          ++count_non_assigned;
        }
        lastSize.at(i) = x[i].size();
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

  virtual ~CompactTable() = default;
  
private:
  
  // void fill_row_map() {
  //   int row_cnt = 0;
  //   for (int j = 0; j < x.size(); j++) {
  //     Int::ViewValues<Int::IntView> i(x[j]);
  //     while (i()) {
  //       rmap_entry entry(key_of(j, i.val()), row_cnt);
  //       row_map.insert(entry);
  //       ++i; ++row_cnt;
  //     }
  //   }
  // }

  // void print_row_map() {
  //   for (int j = 0; j < x.size(); j++) {
  //     Int::ViewValues<Int::IntView> i(x[j]);
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
void extensional2(Home home, const IntVarArgs& x, const TupleSet& t) {
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

  GECODE_ES_FAIL(CompactTable::post(home,vx,t));
  
}


