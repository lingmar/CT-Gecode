#include <gecode/int.hh>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
#include <unordered_map>

//#define DEBUG 1

using namespace Gecode;
using namespace Gecode::Int;
using namespace std;

typedef Support::BitSetBase BitSet;

typedef uint64_t word_t;
typedef string mapkey;
typedef unordered_map<mapkey, int> rmap;
typedef pair<mapkey, int> rmap_entry;

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

int nprops = 0;

// Class for maintaining the supported tuples
class SparseBitSet {
  vector<word_t> words;
  vector<word_t> mask;

  BitSet* words_bs;
  BitSet* mask_bs;
  
  vector<int> index; // type?
  int limit;
  unsigned int nbits;

public:
  SparseBitSet(Home home, unsigned int _nbits) {
    nbits = _nbits;
    unsigned int nwords = required_words(nbits);

    Region r(home);
    
    words_bs = r.alloc<BitSet>(nwords);
    mask_bs = r.alloc<BitSet>(nwords);
    
    // Initialise words
    for (int i = 0; i < nwords; i++) {
      words.push_back(~0ULL); // Set all bits to 1
    }

    // Initialise mask
    for (int i = 0; i < nwords; i++) {
      mask.push_back(0ULL);
    }
    
    /// Initialise index
    for (int i = 0; i < nwords; i++) {
      index.push_back(i);
    }
    /// Limit is initially highest index
    limit = nwords - 1;
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
      mask.at(offset) = 0ULL;
    }
  }

  /// Reverse bits in mask
  void reverse_mask() {
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      mask.at(offset) = ~mask.at(offset);
    }
  }

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
  
  /// Print bit-set for simple debugging
  void print() {
    cout << "words: ";
    for (int i = 0; i < required_words(nbits); i++) {
      //cout << words.at(i) << endl;
      for (int j = 0; j < BITS_PER_WORD; j++) {
        word_t b = (words.at(i) >> j) & 1ULL;
        cout << b << " ";
      }
      cout << endl;
    }

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
  
  void print(ostream& os=cout) const {
    cout << "size: " << size << endl;
    cout << "atoms_per_row: " << atoms_per_row << endl;
    for (int i = 0; i < size/atoms_per_row; i++) {
      for (int j = 0; j < atoms_per_row; j++) {
        for (int k = 0; k < BITS_PER_WORD; k++) {
          os << get(i, j*BITS_PER_WORD + k) << " ";
        }
        os << endl;
      }
    }
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
  // Row map for support entries
  rmap row_map;
  // Last sizes
  vector<int> lastSize;
  // nprop
  int nprop;
  
public:
  // Create propagator and initialize
  CompactTable(Home home,
               ViewArray<IntView>& x0, 
               TupleSet t0,
               int domsum)
    : Propagator(home), x(x0), currTable(home, t0.tuples()),
      supports(domsum, t0.tuples())
  {
    Region r(home);
    
    nprop = nprops++;
    x.subscribe(home,*this,PC_INT_DOM);

    fill_row_map();
    // Initalise supports
    int no_tuples = init_supports(home, t0);

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
    supports.print();
    cout << "currTable: " << endl;
    currTable.print();    
#endif // DEBUG
  }
  
  int rowno(int var, int val) {
    return row_map.at(key_of(var, val));
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
      supports(p.supports),
      row_map(p.row_map),
      lastSize(p.lastSize) {
#ifdef DEBUG
    cout << "copy constructor" << endl;
#endif // DEBUG
    x.update(home,share,p.x);
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
        domsum += x[i].size();
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
    
    //    return home.ES_SUBSUMED(*this);
#ifdef DEBUG
    cout << "before update for prop " << nprop << endl;
    currTable.print();
    updateTable();
    cout << "after update for prop " << nprop << endl;
    currTable.print();
#endif // DEBUG

    
    updateTable();
    if (currTable.is_empty()) {
      return ES_FAILED;
    }
#ifdef DEBUG
    if (nprop == 50) {
      cout << "nr 50" << endl;
    }
#endif // DEBUG
    return filterDomains(home);
  }

  void updateTable() {
    for (int i = 0; i < x.size(); i++) {
      if (lastSize.at(i) == x[i].size()) {
        continue;
      }
      lastSize.at(i) = x[i].size();
      currTable.clear_mask();
      Int::ViewValues<Int::IntView> it(x[i]);
      while (it()) {
        currTable.add_to_mask(supports.get_row(rowno(i,it.val())));
        ++it;
      }
      currTable.intersect_with_mask();
      if (currTable.is_empty()) {
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
  
  void fill_row_map() {
    int row_cnt = 0;
    for (int j = 0; j < x.size(); j++) {
      Int::ViewValues<Int::IntView> i(x[j]);
      while (i()) {
        rmap_entry entry(key_of(j, i.val()), row_cnt);
        row_map.insert(entry);
        ++i; ++row_cnt;
      }
    }
  }

  void print_row_map() {
    for (int j = 0; j < x.size(); j++) {
      Int::ViewValues<Int::IntView> i(x[j]);
      while (i()) {
        cout << "(" << j << ", " << i.val() << ") with key value: " <<
          key_of(j, i.val()) <<
          " is at row " <<
          row_map.at(key_of(j, i.val())) << endl;
        ++i;
      }
    }
  }

  mapkey key_of(int var, int val) {
    char buf[2048];
    sprintf(buf, "%d%d", var, val);
    mapkey key(buf);
    return key;
  }
};

// Post the table constraint
void extensional2(Home home, const IntVarArgs& x, const TupleSet& t) {
  // Never post a propagator in a failed space
  if (home.failed()) return;
  // Set up array of views for the coordinates
  ViewArray<IntView> vx(home,x);

  //  cout << "Post extensional2" << endl;

  //home.fail();
  
  // If posting failed, fail space
  if (CompactTable::post(home,vx,t) != ES_OK)
    home.fail();
}


