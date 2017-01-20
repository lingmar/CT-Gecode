#include <stdint.h>
#include <iostream>
#include <assert.h>

using namespace std;

typedef bool word_t;

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
    /// Set first n bits
    set_bits(nbits);
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
      clear_mask_word(offset);
    }
  }

  /// Reverse bits in mask
  void reverse_mask() {
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      reverse_mask_word(offset);
    }
  }

  /// Add bits to mask
  void add_to_mask(word_t* m) {
    for (int i = 0; i <= limit; i++) {
      int offset = index[i];
      mask[offset] = (mask[offset] || m[offset]);
    }
  }

  /// Intersect words with mask
  void intersect_with_mask() {
    for (int i = limit; i >= 0; i--) {
      int offset = index[i];
      word_t w = words[offset] && mask[offset];
      if (w != words[offset]) {
        words[offset] = w;
        if (w == false) {
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
      if (words[offset] && m[offset] != false) {
        return offset;
      }
    }
    return -1;
  }
  
  /// Print bit-set for simple debugging
  void print() {
    cout << "words: ";
    for (int i = 0; i < nbits; i++) {
      cout << words[i] << " ";
    }
    cout << endl;
    cout << "mask: ";
    for (int i = 0; i < nbits; i++) {
      cout << mask[i] << " ";
    }
    cout << endl;
    cout << "index: ";
    for (int i = 0; i < required_words(nbits); i++) {
      cout << index[i] << " ";
    }
    cout << endl;
    cout << "limit: ";
    cout << limit << endl;;
      
  }


private:
  void set_bits(unsigned int n) {
    assert(n <= nbits);
    for (int i = 0; i < n; i++) {
      words[i] = true;
    }
  }

  void set_word(unsigned int idx, bool state) {
    assert(idx <= limit);
    words[idx] = state;
  }

  void clear_mask_word(unsigned int idx) {
    assert(idx <= limit);
    mask[idx] = false;
  }

  void reverse_mask_word(unsigned int idx) {
    assert(idx <= limit);
    mask[idx] = !mask[idx];
  }
  
  unsigned int required_words(unsigned int nbits) {
    return nbits;
  }
  
};

int main(int argc, char *argv[]) {
  SparseBitSet bs(10);
  cout << "Initial bits" << endl;
  bs.print();
  bs.clear_mask();
  cout << "After clearing mask" << endl;
  bs.print();
  cout << "After reversing mask" << endl;
  bs.reverse_mask();
  bs.print();
  cout << "Restored mask" << endl;
  bs.reverse_mask();
  bs.print();

  word_t* m1 = new word_t[10];
  m1[0] = true;  m1[1] = true;  m1[2] = true;  m1[3] = true;  m1[4] = true;
  m1[5] = true;  m1[6] = true;  m1[7] = true;  m1[8] = false;  m1[9] = true;
  //cout << "Expected mask: 1 0 1 0 1 1 1 1 0 1" << endl;
  bs.add_to_mask(m1); bs.print();

  //cout << "Expected words: 1 0 1 0 1 1 1 1 0 1" << endl;
  //bs.intersect_with_mask(); bs.print();
    
  // cout << "Expected mask: 1 1 1 0 1 1 1 1 0 1" << endl;
  m1[1] = false;
  bs.add_to_mask(m1); bs.print();

  // cout << "Expected words: 1 0 1 0 1 1 1 1 0 1" << endl;
  // bs.intersect_with_mask(); bs.print();

  // cout << "Expected mask: 1 1 1 1 1 1 1 1 0 1" << endl;
  // m1[3] = true;
  // bs.add_to_mask(m1); bs.print();
  
  // cout << "Expected words: 1 0 1 0 1 1 1 1 0 1" << endl;
  // bs.intersect_with_mask(); bs.print();
  
  // cout << "Exptected words: 1 0 1 0 1 1 1 1 0 1" << endl;
  // bs.intersect_with_mask(); bs.print();

  // cout << "Exptected: 0" << endl;
  // cout << bs.intersect_index(m1) << endl;

  
  return 0;
}
