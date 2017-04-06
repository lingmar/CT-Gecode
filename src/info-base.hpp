#include <gecode/int.hh>

using namespace Gecode;

//#define DEBUG

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif
  
/**
 * Abstract base class for classes providing supports and 
 * residues information
 */
class InfoBase {
protected:
  /// Support bits (valid tuple indices)
  BitSet* supports;
  /// Residues
  unsigned int* residues;
  /// Number of values
  int nvals;
public:
  /// Default constructor
  InfoBase(void) {}
  /// Copy constructor
  InfoBase(const InfoBase& ib)
    : nvals(ib.nvals)
  {
    supports = heap.alloc<BitSet>(nvals);
    residues = heap.alloc<unsigned int>(nvals);
    for (int i = 0; i < nvals; i++) {
      supports[i].init(heap,ib.supports[i].size());
      supports[i].copy(ib.supports[i].size(), ib.supports[i]);
      residues[i] = ib.residues[i];
    }
  }
  virtual BitSet get_supports(int val) = 0;
  virtual unsigned int get_residue(int val) = 0;
  virtual void set_residue(int val, unsigned int r) = 0;
  virtual void init(const BitSet* supports,
                    const unsigned int* residues,
                    int init_min, int max,
                    int nsupports, int offset,
                    BitSet dom) = 0;
};

class InfoArray : public InfoBase {
private:
  /// Initial minimum value for x
  int min;
public:
  /// Default constructor
  InfoArray(void) {}
  /// Copy constructor
  InfoArray(const InfoArray& s)
    : InfoBase(s), min(s.min) {}
  
  virtual BitSet get_supports(int val) {
    return supports[val - min];
  }

  virtual unsigned int get_residue(int val) {
    return residues[val - min];
  }

  virtual void set_residue(int val, unsigned int r) {
    residues[val - min] = r;
  }
  
  virtual void init(const BitSet* s,
                    const unsigned int* r,
                    int init_min, int max,
                    int nsupports, int offset,
                    BitSet dom) {
            // Number of bitsets
        nvals = static_cast<unsigned int>(max - init_min + 1);
        DEBUG_PRINT(("init supports, nsupports = %d, nvals = %d\n",
                     nsupports,nvals));

        // Allocate memory and initialise
        supports = heap.alloc<BitSet>(nvals);
        residues = heap.alloc<unsigned int>(nvals);
        for (int i = 0; i < nvals; i++) {
          assert(nsupports <= s[offset + i].size());
          supports[i].init(heap,nsupports);
          supports[i].copy(nsupports,s[i + offset]);

          residues[i] = r[i + offset];
        }
        min = init_min;
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
    /// Allocate space a hash table with room for \a pop elements
    void allocate(int pop) {
      DEBUG_PRINT(("Allocate hash %d\n",pop));
      size=2;
      while (size <= 2*pop)
        size *= 2;

      table = heap.alloc<HashNode>(size);
      mask = size-1;
      factor = 0.618 * size;
      for (int i = 0; i < size; i++) 
        table[i].value = -1; 	/* mark as free */
    }
    /// Copy constructor
    HashTable(const HashTable& h)
      : mask(h.mask), factor(h.factor), size(h.size)
    {
      DEBUG_PRINT(("Copy hash table\n"));
      table = heap.alloc<HashNode>(size);
      for (int i = 0; i < size; i++)
        table[i] = h.table[i];
    }
    
    /// Insert element into hash table
    void insert(int key, int value) {
      DEBUG_PRINT(("Insert {%d,%d}\n",key,value));
      long t0 = key*factor;
      int inc=0;
      while (1) {
        HashNode* hnode = &table[t0 & mask];
        if (hnode->value == -1) {	/* value=-1 means free */
          hnode->key = key;
          hnode->value = value;
          return;
        }
        inc++;
        t0 += inc;
      }
    }
    /// Get value of element with key \a key
    int get(int key) const {
      long t0 = key*factor;
      int inc=0;
  
      while (1) {
        HashNode* hnode = &table[t0 & mask];
        if (hnode->key == key) {
          DEBUG_PRINT(("Access {%d,%d}\n",key,hnode->value));
          return hnode->value;
        }
        inc++;
        t0 += inc;
      }
    }

    static void test() {
      long keys[] = {43, 24, 17, 77, 25, 50, 94, 49, 22, 44, 10, 31, 68, 100, 2, 99, 3, 85, 9, 87, 73, 26, 63, 8, 67, 95, 92, 42, 1, 47, 37, 33, 81, 54, 39, 27, 56, 40, 86, 4, 35, 21, 13, 7, 79, 28, 72, 46, 58, 12, 41, 69, 6, 32, 11, 36, 71, 48, 29, 78, 96, 83, 5, 30, 45, 88, 84, 18, 93, 15, 89, 53, 19, 65, 60, 66, 34, 59, 70, 16, 23, 74, 20, 61, 55, 64, 76, 75, 82, 57, 80, 62, 51, 97, 14, 52, 91, 98, 90, 38};

      HashTable h;
      h.allocate(100);
      int i;
      
      printf("index sequence: ");
      for (i=0; i<=h.mask; i++)
        printf("%ld ", (h.factor*i) & h.mask);
      printf("\n");
      
      for (i=0; i<100; i++)
        h.insert(keys[i], 101-keys[i]);
      for (i=0; i<100; i++)
        printf("hash[%ld] = %d\n", keys[i], h.get(keys[i]));
    }

    
  };
  /// Hash table for indexing supports and residues
  HashTable index_table;
  
public:
  /// Default constructor
  InfoHash(void) {}
  /// Copy constructor
  InfoHash(const InfoHash& ih)
    : InfoBase(ih), index_table(ih.index_table) {}
  
  virtual BitSet get_supports(int val) {
    return supports[index_table.get(val)];
  }

  virtual unsigned int get_residue(int val) {
    return residues[index_table.get(val)];
  }

  virtual void set_residue(int val, unsigned int r) {
    residues[index_table.get(val)] = r;
  }

  virtual void init(const BitSet* s,
                    const unsigned int* r,
                    int init_min, int max,
                    int nsupports, int offset,
                    BitSet dom) {
    DEBUG_PRINT(("init hash\n"));
    //HashTable::test();
    // Initial domain size
    nvals = dom.nset();
    // Allocate memory
    index_table.allocate(nvals);
    supports = heap.alloc<BitSet>(nvals);
    residues = heap.alloc<unsigned int>(nvals);

    int count = 0;
    for (int i = 0; i < dom.size(); i++) {
      if (dom.get(i)) {
        assert(nsupports <= s[offset + i].size());
        // Initialise and copy nsupports bits
        supports[count].init(heap,nsupports);
        supports[count].copy(nsupports,s[i+offset]);
        // Set residue and save index to table
        residues[count] = r[i+offset];
        index_table.insert(i + init_min,      /** actual value is i+init_min **/
                           count);
        ++count;
      }
    }
    DEBUG_PRINT(("Finish init hash\n"));
  }
};
