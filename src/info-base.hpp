#include <gecode/int.hh>
#include <signal.h>
#include "bitset.hpp"

using namespace Gecode;

//#define DEBUG

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif
  
/**
 * Abstract base class for classes providing supports
 * information for a variable
 */
class InfoBase {
protected:
  /// Support bits (valid tuple indices)
  BitSet* supports;
public:
  /// Number of values
  int nvals;
  /// Default constructor
  InfoBase(void) {}
  /// Copy constructor
  InfoBase(const InfoBase& ib) {
    supports = heap.alloc<BitSet>(ib.nvals);
    copy(ib);
  }
  /// Copy \a ib
  forceinline void
  copy(const InfoBase& ib) {
    nvals = ib.nvals;
    for (int i = 0; i < nvals; i++) {
      if (!ib.supports[i].empty()) {
        supports[i].init(heap,ib.supports[i].size());
        supports[i].copy(ib.supports[i].size(), ib.supports[i]);
      }
    }
  }
  /// Allocate with allocator \a for \a n values
  forceinline virtual void
  allocate(int n) {
    nvals = n;
    supports = heap.alloc<BitSet>(nvals);
  }
  /// Abstract functions
  forceinline virtual const BitSet&
  get_supports(int val) {
    assert(row(val) >= 0);
    return supports[row(val)];
  }
  
  forceinline virtual const BitSet&
  get_supports_raw(int row) {
    assert(row >= 0);
    return supports[row];
  }
  
  template<class View>
  void init(const BitSet* supports,int nsupports, int offset,View x);

  virtual int row(int val) = 0;

  forceinline virtual ~InfoBase(void) {
    for (int i = nvals; i--; ) {
      (void) supports[i].dispose(heap);
    }
    heap.rfree(supports);
  }
    
 };

class InfoArray : public InfoBase {
private:
  /// Initial minimum value for x
  int min;
  /// Initial maximum value for x
  int max;
public:
  /// Default constructor
  InfoArray(void) {}
  /// Copy constructor
  InfoArray(const InfoArray& s)
    : InfoBase(s), min(s.min), max(s.max) {}

  // virtual const BitSet& get_supports(int val) {
  //   return suports[row(val)];
  // }
  
  template<class View>
  forceinline void
  init(const BitSet* s,int nsupports, int offset,View x) {
    min = x.min();
    max = x.max();
    // Number of bitsets
    nvals = static_cast<unsigned int>(max - min + 1);
    // Allocate memory and initialise
    supports = heap.alloc<BitSet>(nvals);
    for (int i = 0; i < nvals; i++) {
      if (!s[i + offset].empty()) { // Skip empty sets
        assert(nsupports <= s[offset + i].size());
        supports[i].init(heap,nsupports);
        supports[i].copy(nsupports,s[i + offset]);
      }
    }
  }

  forceinline virtual void
  allocate(int n) {
    InfoBase::allocate(n);
  }

  forceinline virtual int
  row(int val) {
    //printf("val - min = %d - %d = %d\n",val,min,val-min );
    return val >= min && val <= max ? val - min : -1;
  }  
};

class InfoHash : public InfoBase {
private:
  class HashTable {
  public:
    typedef struct HashNode {
      int key;
      int value; 
    } HashNode;
    /// Table with entries
    HashNode* table;
    /// Mask
    long mask;
    /// Factor
    long factor;
    /// Size
    int size;
    /// Default constructor
    HashTable() {}
    /// Allocate space for a hash table with room for \a pop elements
    forceinline void
    allocate(int pop) {
      size=2;
      while (size <= 2*pop)
        size *= 2;

      DEBUG_PRINT(("Allocate hash with pop %d, size %d\n",pop,size));
      table = heap.alloc<HashNode>(size);
      mask = size-1;
      factor = 0.618 * size;
      for (int i = 0; i < size; i++) 
        (&table[i])->value = -1; 	/* mark as free */
    }
    /// Copy constructor
    forceinline
    HashTable(const HashTable& h)
      : mask(h.mask), factor(h.factor), size(h.size)
    {
      DEBUG_PRINT(("Copy hash table\n"));
      table = heap.alloc<HashNode>(size);
      for (int i = 0; i < size; i++)
        table[i] = h.table[i];
    }
    
    /// Insert element into hash table
    forceinline void
    insert(int key, int value) {
      long t0 = key*factor;
      int inc=0;
      while (1) {
        HashNode* hnode = &table[t0 & mask];
        if (hnode->value == -1) {	/* value=-1 means free */
          DEBUG_PRINT(("Insert {%d,%d} at row %ld\n",key,value,(t0 & mask)));
          hnode->key = key;
          hnode->value = value;
          return;
        }
        inc++;
        t0 += inc;
      }
    }
    /// Get value of element with key \a key
    forceinline int
    get(int key) const {
      DEBUG_PRINT(("Looking for key %d\n", key));
      long t0 = key*factor;
      int inc=0;
  
      while (1) {
        HashNode* hnode = &table[t0 & mask];
        if (hnode->key == key || hnode->value == -1) {
          DEBUG_PRINT(("Access {%d,%d} at row %ld\n",key,hnode->value, (t0 & mask)));
          return hnode->value;
        }
        inc++;
        t0 += inc;
      }
    }
    /// Print
    forceinline void
    print() {
      for (int i = 0; i < size; i++) {
        DEBUG_PRINT(("%d: {%d,%d}\n",i,table[i].key,table[i].value));
      }
    }
    ~HashTable(void) {
      heap.rfree(table);
    }
  };
  /// Hash table for indexing supports
  HashTable index_table;
  
public:
  /// Default constructor
  forceinline
  InfoHash(void) {}
  /// Copy constructor
  forceinline
  InfoHash(const InfoHash& ih)
    : InfoBase(ih), index_table(ih.index_table) {
    DEBUG_PRINT(("Copy InfoHash\n"));
  }

  // virtual const BitSet& get_supports(int val) {
  //   return supports[row(val)];
  // }  
  
  forceinline virtual void
  allocate(int n) {
    InfoBase::allocate(n);
    index_table.allocate(n);
  }

  template<class View>
  forceinline void
  init(const BitSet* s,int nsupports, int offset,View x) {
    // Initial domain size
    nvals = x.size();
    // Allocate memory
    index_table.allocate(nvals);
    supports = heap.alloc<BitSet>(nvals);

    int count = 0;
    int diff = x.min();

    Int::ViewValues<View> it(x);
    while (it()) {
      assert(nsupports <= s[offset + it.val() - diff].size());

      // Initialise and save index to table
      supports[count].init(heap,nsupports);
      supports[count].copy(nsupports,s[it.val() + offset - diff]);
      index_table.insert(it.val(),count);
      
      ++count;
      ++it;
    }
    index_table.print();
  }

  forceinline virtual int
  row(int val) {
    return index_table.get(val);
  }
  
};
