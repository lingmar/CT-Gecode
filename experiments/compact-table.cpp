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
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <assert.h>
#include <unordered_map>

#define BITS_PER_WORD 64

using namespace Gecode;
using namespace Gecode::Int;
using namespace std;

typedef uint64_t word_t;
typedef string mapkey;
typedef unordered_map<mapkey, int> rmap;
typedef pair<mapkey, int> rmap_entry;

class SparseBitSet {
  vector<word_t> words;
  vector<word_t> mask;
  vector<int> index; // type?
  int limit;
  unsigned int nbits;

public:
  SparseBitSet(unsigned int _nbits) {
    nbits = _nbits;
    unsigned int nwords = required_words(nbits);

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
  SparseBitSet(SparseBitSet& sbs) :
    words(sbs.words),
    mask(sbs.mask),
    index(sbs.index),
    limit(sbs.limit),
    nbits(sbs.limit) {
  }
  
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

    cout << "mask: ";
    for (int i = 0; i < required_words(nbits); i++) {
      //cout << mask.at(i) << endl;
      for (int j = 0; j < BITS_PER_WORD; j++) {
        word_t b = (mask.at(i) >> j) & 1ULL;
        cout << b << " ";
      }
      cout << endl;
    }
   
    cout << "index: ";
    for (int i = 0; i < required_words(nbits); i++) {
      cout << index.at(i) << " ";
    }
    cout << endl;
    cout << "limit: ";
    cout << limit << endl;;
      
  }

  unsigned int required_words(unsigned int nbits) {
    if (nbits == 0) {
      return 0;
    } else {
      return (nbits - 1) / BITS_PER_WORD + 1;
    }
  }

};


struct BitSet {
#define Toggle(a, i) (a) |= (1UL << (i))
#define Clear(a, i) (a) &= ~(1UL << (i))
#define Get(a, i) (((a) >> (i)) & 1UL)
  
  vector<word_t> atoms;
  size_t size;
  size_t atoms_per_row;

  BitSet(int rows, int cols)
    : atoms_per_row(ceil((double)cols/BITS_PER_WORD)) {
    size = required_ints(rows);
    for (int i = 0; i < size; i++) {
      atoms.push_back(0UL);
    }
  }
  
  BitSet(const BitSet& other)
    : atoms(other.atoms)
    , size(other.size)
    , atoms_per_row(other.atoms_per_row) {}

  int required_ints(int rows) {
    return atoms_per_row*rows;
  }

  void set(int r, int c, bool state) {
    if (state) {
      Toggle(atoms.at(atom_idx(r,c)), bit_idx(c));
    } else {
      Clear(atoms.at(atom_idx(r,c)), bit_idx(c));
    }
  }

  bool get(int r, int c) const {
    return Get(atoms.at(atom_idx(r,c)), bit_idx(c)) == 1UL;
  }

  void print(ostream& os=cout) const {
    cout << "size: " << size << endl;
    cout << "atoms_per_row: " << atoms_per_row << endl;
    for (int i = 0; i < size/atoms_per_row; i++) {
      for (int j = 0; j < BITS_PER_WORD; j++) {
        os << get(i,j) << " ";
      }
      os << endl;
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

void extensional2(Home home, const IntVarArgs& x, const TupleSet& t);

// The no-overlap propagator
class CompactTable : public Propagator {
protected:  
  // The variables
  ViewArray<IntView> x;
  // The table with possible combinations of values
  TupleSet ts;
  // Valid tuples
  SparseBitSet currTable;
  // Supported tuples
  BitSet supports;
  // Row map for support entries
  rmap row_map;
  
public:
  // Create propagator and initialize
  CompactTable(Home home,
               ViewArray<IntView>& x0, 
               TupleSet t0)
    : Propagator(home), x(x0), ts(t0), currTable(t0.tuples()),
      supports(x0[0].size() + x[1].size() + x[2].size(), t0.tuples())
  {
    x.subscribe(home,*this,PC_INT_DOM);

    fill_row_map();
    //    print_row_map();
    init_supports();
    //cout << "supports: " << endl;
    //supports.print();
  }

  int rowno(int var, int val) {
    return row_map.at(key_of(var, val));
  }
  
  void init_supports() {
    for (int i = 0; i < ts.tuples(); i++) {
      cout << "tuple[" << i << "] = ";
      bool supported = true;
      for (int j = 0; j < ts.arity(); j++) {
        cout << ts[i][j] << " ";
        if (!x[j].in(ts[i][j])) {
          supported = false;
          cout << "Value " << ts[i][j] << " not supported for variable " << j << endl;
          break;
        }
      }
      if (supported) {
        // Set tuple as valid
        cout << "All values supported" << endl;
        set_tuple(i);
      }
    } 
    cout << "supports" << endl;
  }

  void set_tuple(int t) {
    assert(t <= ts.tuples());
    for (int i = 0; i < ts.arity(); i++) {
      supports.set(rowno(i, ts[t][i]), t, true);
    }
  }
  
  // Copy constructor during cloning
  CompactTable(Space& home, bool share, CompactTable& p)
    : Propagator(home,share,p), ts(p.ts), currTable(p.currTable), supports(p.supports) {
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
    // Binary case
    GECODE_ME_CHECK(x.size() == 2);
    
    // Only if there is something to propagate
    if (x.size() > 1)
      (void) new (home) CompactTable(home,x,t);
    return ES_OK;
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
    //return home.ES_SUBSUMED(*this);
    supports.print();
    currTable.print();
    return ES_NOFIX;
  }
    
  // Dispose propagator and return its size
  virtual size_t dispose(Space& home) {
    x.cancel(home,*this,PC_INT_BND);
    // TODO: dispose t?
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }
  
private:
  
  void fill_row_map() {
    int row_cnt = 0;
    for (int j = 0; j < x.size(); j++) {
      Int::ViewValues<Int::IntView> i(x[j]);
      while (i()) {
        cout << "val x[" << j << "] " << i.val() << " ";
        rmap_entry entry(key_of(j, i.val()), row_cnt);
        row_map.insert(entry);
        ++i; ++row_cnt;
      }
      cout << endl;
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
  // If posting failed, fail space
  if (CompactTable::post(home,vx,t) != ES_OK)
    home.fail();
}
