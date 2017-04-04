#include <gecode/kernel.hh>
#include "bitset.hpp"
#include "linked-list.cpp"
#include "hash-table.cpp"

//#define MEMOPT 1

//#define HASH 1

#define PRIME_1 7
#define PRIME_2 11

using namespace Gecode;
using namespace std;

class SharedSupports : public SharedHandle {
protected:
  class Supports : public SharedHandle::Object {
  public:
    /// Support bits
    BitSet* supports;
    /// Starting index
    unsigned int* start_idx;
    /// Starting value
    unsigned int* start_val;
    /// Number of rows
    unsigned int domsize;
    /// Number of bits in each row
    unsigned int nsupports;
    /// Number of variables
    unsigned int vars;
    /// Default constructor (empty set)
    Supports(void);
    /// Copy \a s
    Supports(const Supports& s);
    /// Allocate supports for \a d rows, \a variables and \a n bits
    void allocate_supports(unsigned int d,unsigned int v,unsigned int n);
    /// Row number for variable \a var and value \a val
    unsigned int row(unsigned int var, int val) const;
    /// Copy function
    virtual Object* copy(void) const;
    /// Destructor
    virtual ~Supports(void);
#ifdef HASH
    // class HashEntry {
    // public:
    //   int var;
    //   int val;
    //   int row;
    //   HashEntry* next;
    // };
    // /// Hash table for indexing
    // HashEntry* idx_table;
    /// Fill table with domains
    void fill(BitSet* dom, int sz, int min, int max);
    /// Get the row for variable \a var, value \a val
    unsigned int rowno(int var, int val);
    /// Remove element
    void remove(int var, int val);
    /// Add hash element
    void add(int var, int val, int row);
    /// Change row for hash entry
    // void set(int var, int val, int row);
    HashTable idx_table;
#endif // HASH
  };
public:
  /// Default constructor (yields empty set)
  SharedSupports(void);
  /// Copy \a s
  SharedSupports(const SharedSupports& s);
  /// Assignment operator
  SharedSupports& operator =(const SharedSupports& si);
  /// Get bit set for variable i, value j
  BitSet& get(unsigned int i, int j) const;
  /// Allocate supports for \a d rows, \a variables and \a n bits
  void init_supports(unsigned int d, unsigned int v, unsigned int n);
  /// Set start index for variable \a i to \a idx
  void set_start_idx(unsigned int i, unsigned int idx);
  /// Set start value for variable \a i to \a val
  void set_start_val(unsigned int i, unsigned int val);
  /// Get start index for variable \a i
  unsigned int get_start_idx(unsigned int i) const;
  /// Get start index for variable \a i
  unsigned int get_start_val(unsigned int i) const;
  
  /// Print bit set for variable \a i and value \a
  void print(unsigned int i,unsigned int j) const;
  /// Row number for variable \a var and value \a val
  unsigned int row(unsigned int var, int val) const;
#ifdef HASH
  /// Fill index hash table
  void fill(BitSet* dom, int sz, int min, int max);
#endif // HASH
  void update(Space& home, bool share, SharedHandle& sh) {
    SharedHandle::update(home,share,sh);
  }
  /// Destructor
  ~SharedSupports(void);
};

forceinline
SharedSupports::Supports::Supports(void)
  : supports(NULL), start_idx(NULL), domsize(0),
    nsupports(0), vars(0) {
}

forceinline
SharedSupports::Supports::Supports(const Supports& s)
  : domsize(s.domsize), vars(s.vars), nsupports(s.nsupports)
{
#ifdef HASH
  idx_table.init(domsize);
#endif // HASH
    
  supports = heap.alloc<BitSet>(domsize);
  start_idx = heap.alloc<unsigned int>(vars);
  start_val = heap.alloc<unsigned int>(vars);
  
#ifdef MEMOPT
  // Allocate a chunk of memory to store the bit sets
  unsigned int bases_per_bs = Support::BitSetData::data(nsupports);
  Support::BitSetData* mem =
    heap.alloc<Support::BitSetData>((nsupports+1) * bases_per_bs);
  for (unsigned int i = 0; i < domsize; i++) {
    supports[i].init(&mem[i * bases_per_bs],nsupports,false);
    supports[i].copy(nsupports,s.supports[i]);
  }
#else
  for (unsigned int i = 0; i < domsize; i++) {
    supports[i].Support::BitSetBase::init(heap,nsupports,false);
    supports[i].copy(nsupports,s.supports[i]);
  }
#endif // MEMOPT
  for (unsigned int i = 0; i < vars; i++) {
    start_idx[i] = s.start_idx[i];
    start_val[i] = s.start_val[i];
  }
}

forceinline void
SharedSupports::Supports::allocate_supports(unsigned int d,unsigned int v,unsigned int n) {
  domsize = d;
  vars = v;
  nsupports = n;
  supports = heap.alloc<BitSet>(domsize);
  start_idx = heap.alloc<unsigned int>(vars);
  start_val = heap.alloc<unsigned int>(vars);
#ifdef HASH
  idx_table.init(domsize);
#endif // HASH
#ifdef MEMOPT
  // Allocate a chunk of memory to store the bit sets
  unsigned int bases_per_bs = Support::BitSetData::data(nsupports);
  Support::BitSetData* mem =
    heap.alloc<Support::BitSetData>(nsupports * bases_per_bs);
  for (unsigned int i = 0; i < d; i++) {
    //std::cout << "allocate " << i << "/" << d << std::endl;
    supports[i].init(&mem[i * bases_per_bs],nsupports,false);
  }
#else
  for (unsigned int i = 0; i < d; i++) {
    supports[i].Support::BitSetBase::init(heap,nsupports,false);
  }
#endif // MEMOPT
}

forceinline unsigned int
SharedSupports::Supports::row(unsigned int var, int val) const {
  return start_idx[var] + val - start_val[var];
}

forceinline SharedSupports::Object*
SharedSupports::Supports::copy(void) const {
  return new Supports(*this);
}

forceinline
SharedSupports::Supports::~Supports(void) {
  //  heap.rfree(supports);
  //heap.rfree(start_idx);
  //heap.rfree(start_val);
  // TODO: bitsets
}

#ifdef HASH

forceinline void
SharedSupports::Supports::fill(BitSet* dom, int sz, int min, int max) {
  unsigned int row;
  for (int i = 0; i < sz; i++) {
    for (unsigned int j=Gecode::Support::BitSetData::data(sz+1); j--; ) {
      if (dom[i].get(j)) {
        add(i,j+min,row);
        row++;
      }
    }
  }
}

forceinline unsigned int
SharedSupports::Supports::rowno(int var, int val) {
  Key key = {var,val};
  return idx_table.hash(key);
}

forceinline void
SharedSupports::Supports::remove(int var, int val) {
  Key key = {var,val};
  idx_table.remove(key);
}

forceinline void
SharedSupports::Supports::add(int var, int val, int row) {
  Key key = {var,val};
  Item* item = heap.alloc<Item>(1);
  item->key = key;
  item->row = row;
  item->next = NULL;
  idx_table.insert(item);
}

#endif // HASH

/**
 * SharedSupports
 **/
forceinline
SharedSupports::SharedSupports(void)
  : SharedHandle(new Supports()) {}

forceinline
SharedSupports::SharedSupports(const SharedSupports& si)
  : SharedHandle(si) {}

forceinline SharedSupports&
SharedSupports::operator =(const SharedSupports& si) {
  return static_cast<SharedSupports&>(SharedHandle::operator =(si));
}

forceinline BitSet&
SharedSupports::get(unsigned int i, int j) const {
  const SharedSupports::Supports* s =
    static_cast<const SharedSupports::Supports*>(object());
  return s->supports[s->row(i,j)];
}

forceinline void
SharedSupports::init_supports(unsigned int d, unsigned int v, unsigned int n) {
  static_cast<SharedSupports::Supports*>(object())->allocate_supports(d,v,n);
}

forceinline void
SharedSupports::set_start_idx(unsigned int i, unsigned int idx) {
  static_cast<SharedSupports::Supports*>(object())->start_idx[i] = idx;
}

forceinline void
SharedSupports::set_start_val(unsigned int i, unsigned int val) {
  static_cast<SharedSupports::Supports*>(object())->start_val[i] = val;
}

forceinline unsigned int
SharedSupports::get_start_idx(unsigned int i) const {
  return static_cast<SharedSupports::Supports*>(object())->start_idx[i];
}

forceinline unsigned int
SharedSupports::get_start_val(unsigned int i) const {
  return static_cast<SharedSupports::Supports*>(object())->start_val[i];
}

forceinline void
SharedSupports::print(unsigned int i,unsigned int j) const {
  const SharedSupports::Supports* s =
    static_cast<SharedSupports::Supports*>(object());
  s->supports[s->row(i,j)].print();
}

forceinline unsigned int
SharedSupports::row(unsigned int var, int val) const {
  return static_cast<SharedSupports::Supports*>(object())->row(var,val);
}

forceinline
SharedSupports::~SharedSupports(void) {}

#ifdef HASH
// forceinline void
// SharedSupports::Supports::fill(BitSet* dom, int sz, int min, int max) {
//   std::cout << "fill" << std::endl;
//   unsigned int n_elements = static_cast<unsigned int>(max - min + 1);
//   int count = 0;
//   for (int i = 0; i < sz; i++) {
//     std::cout << "i=" << i << std::endl;
//     for (int j = 0; j < n_elements; j++) {
//       std::cout << "j="<< j << std::endl;
//       if (dom[i].get(j)) {
//         std::cout << "set!" << std::endl;
//         add(i,j+min,count);
//         std::cout << "finished setting!" << std::endl;
//       }
//     }
//   }
//   std::cout << "end fill" << std::endl;
// }

// forceinline void
// SharedSupports::Supports::add(int var, int val, int row) {
//   std::cout << "add" << std::endl;
//   int h = (var * PRIME_1 + val * PRIME_2) % domsize;
//   cout << "h=" << h << endl;
//   HashEntry* entry = &idx_table[h];
//   while (entry != NULL) {
//     entry = entry->next;
//   }
//   entry->var = var;
//   entry->val = val;
//   entry->row = row;
//   std::cout << "finish add" << std::endl;
// }

// /// Get the row for variable \a i, value \a j
// forceinline unsigned int
// SharedSupports::Supports::rowno(int i, int j) {
//   int h = (i * PRIME_1 + j * PRIME_2) % domsize;
//   HashEntry* entry = &idx_table[h];
//   while (entry->var != i || entry->val != j) {
//     entry = entry->next;
//   }
//   return entry->row;
// }

// /// Remove element
// void remove(int i, int j);

forceinline void
SharedSupports::fill(BitSet* dom, int sz, int min, int max) {
  return static_cast<SharedSupports::Supports*>(object())->
    fill(dom,sz,min,max);
}

#endif // HASH

