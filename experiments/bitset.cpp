#include <stdint.h>
#include <iostream>
#include <assert.h>

#define BITS_PER_WORD 64

using namespace std;

typedef uint64_t word_t;

class SparseBitSet {
  word_t* words;
  word_t* mask;
  int* index; // type?
  int limit;
  unsigned int nbits;

public:
  SparseBitSet(unsigned int _nbits) {
    nbits = _nbits;
    unsigned int nwords = required_words(nbits);
    /// Initialise words
    words = new word_t[nwords];
    // Fixme: How to initialise the bits?
    for (int i = 0; i < nwords; i++) {
      words[i] = ~0ULL; // Set all bits to 1
    }

    /// Initilise mask
    mask = new word_t[nwords];
    /// Initialise index
    index = new int[nwords];
    for (int i = 0; i < nwords; i++) {
      index[i] = i;
    }
    /// Limit is initially highest index
    limit = nwords - 1;
  }

  /// Return true if bitset is empty, else false
  bool is_empty() const {
    return limit == -1;
  }
  
  /// Clear all bits in mask
  void clear_mask() {
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      mask[offset] = 0ULL;
    }
  }

  /// Reverse bits in mask
  void reverse_mask() {
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      mask[offset] = ~mask[offset];
    }
  }

  /// Add bits to mask
  void add_to_mask(word_t* m) {
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      mask[offset] |= m[offset];
    }
  }

  /// Intersect words with mask
  void intersect_with_mask() {
    for (int i = limit; i >= 0; i--) {
      int offset = index[i];
      word_t w = (words[offset] & mask[offset]);
      if (w != words[offset]) {
        words[offset] = w;
        if (w == 0ULL) {
          index[i] = index[limit];
          index[limit] = offset;
          --limit;
        }
      }
    }
  }

  /* Returns the index of a word where the bit-set
  intersects with m, -1 otherwise */
  int intersect_index(word_t* m) const {
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      if ((words[offset] & m[offset]) != 0ULL) {
        return offset;
      }
    }
    return -1;
  }
  
  /// Print bit-set for simple debugging
  void print() {
    cout << "words: ";
    for (int i = 0; i < required_words(nbits); i++) {
      //cout << words[i] << endl;
      for (int j = 0; j < BITS_PER_WORD; j++) {
        word_t b = (words[i] >> j) & 1ULL;
        cout << b << " ";
      }
      cout << endl;
    }

    cout << "mask: ";
    for (int i = 0; i < required_words(nbits); i++) {
      //cout << mask[i] << endl;
      for (int j = 0; j < BITS_PER_WORD; j++) {
        word_t b = (mask[i] >> j) & 1ULL;
        cout << b << " ";
      }
      cout << endl;
    }
   
    cout << "index: ";
    for (int i = 0; i < required_words(nbits); i++) {
      cout << index[i] << " ";
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

int main(int argc, char *argv[]) {
  SparseBitSet bs(65);
  bs.clear_mask();
  bs.print();
  
  word_t* m = new word_t[2];
  m[0] = 0ULL;
  m[1] = ~0ULL;
  bs.add_to_mask(m);
  
  bs.intersect_with_mask(); bs.print();

  return 0;
}
