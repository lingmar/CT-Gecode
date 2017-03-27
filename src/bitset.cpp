class BitSet : public Gecode::Support::BitSetBase {
public:
  // /// Bit set with space for \a s bits
  // template<class A>
  // BitSet(A& a, unsigned int s, bool set=false);
  // /// Copy bit set \a bs with allocator \a a
  // template<class A>
  // BitSet(A& a, const BitSet& bs);
  

  /// Default constructor (yields empty set)
  BitSet(void);
  /// Initialize for \a sz bits and allocator \a a
  template<class A>
  BitSet(A& a, unsigned int sz, bool setbits);
  /// Copy constructor
  BitSet(const BitSet&);
  ///Assignment operator
  BitSet& operator =(const BitSet&);
  /// Perform "or" with \a a of word index \a i
  void o(BitSet a, unsigned int i);
  /// Perform "and" with \a a of word index \a 
  void a(BitSet a, unsigned int i);
  /// Return "or" of \a a and \a b of word index \a i
  static Gecode::Support::BitSetData o(BitSet a, BitSet b, unsigned int i);
  /// Return "and" of \a a and \a b of word index \a i
  static Gecode::Support::BitSetData a(BitSet a, BitSet b, unsigned int i);
  /// Clear all bits at word index \a i
  void clearword(unsigned int i, bool setbits);
  /// Set a word \a i to \a a
  void setword(Gecode::Support::BitSetData a, unsigned int i);
  /// Get word \a i
  Gecode::Support::BitSetData getword(unsigned int i);
  /// Check if bit set has \a d on index \a i
  bool same(Gecode::Support::BitSetData d, unsigned int i);
  /// Get number of bits per base
  static int get_bpb();
  /// Print bit set
  void print();
};

template<class A>
class NewSparseBitSet {
private:
  /// Allocator
  A& a;
  /// Mask
  BitSet mask;
  /// Index
  unsigned int* index;
  /// Limit
  int limit;
  /// Size
  int sz;
public:
  /// Words (FIXME: why public?)
  BitSet words;
  
  /// Sparse bit set with space for \a s bits and allocator \a a
  NewSparseBitSet(A& a, unsigned int s);
  /// Copy sparse bit set \a sbs with allocator \a a
  NewSparseBitSet(A& a, const NewSparseBitSet& sbs);
  /// Destructor
  //~NewSparseBitSet(void);
  /// Check if sparse bit set is empty
  bool is_empty() const;
  /// Clear the mask
  void clear_mask();
  /// Add bits in \a b to mask
  void add_to_mask(BitSet b);
  /// Intersect words with mask
  void intersect_with_mask();
  /// Get the index of a non-zero intersect with \a b, or -1 if none exists
  int intersect_index(BitSet b);
  /// Print bit set
  void print();
  void foo(sldkfjaösf);
};

/**
 * Sparse bit set
 */
template<class A>
forceinline
NewSparseBitSet<A>::NewSparseBitSet(A& a0, unsigned int s)
  : a(a0), words(a,s,false), mask(a,s,false), sz(s) {
  // Calculate number of required words
  int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
  index = a.template alloc<unsigned int>(nwords);
  limit = nwords - 1;
  for (int i = 0; i <= limit; i++) {
    index[i] = i;
  }
}

template<class A>
forceinline
NewSparseBitSet<A>::NewSparseBitSet(A& a0, const NewSparseBitSet<A>& sbs)
  : a(a0), sz(sbs.sz), limit(sbs.limit), words(a,sbs.sz,false), mask(a,sbs.sz,false) {
  int nwords = sz != 0 ? (sz - 1) / words.get_bpb() + 1 : 0;
  index = a.template alloc<unsigned int>(limit + 1);
  for (int i = 0; i <= limit; i++) {
    index[i] = sbs.index[i];
  }
}

template<class A>
forceinline bool
NewSparseBitSet<A>::is_empty() const {
  return limit == -1;
}

template<class A>
forceinline void
NewSparseBitSet<A>::clear_mask() {
  for (int i = 0; i <= limit; i++) {
    int offset = index[i];
    mask.clearword(offset, false);
  }
}

template<class A>
forceinline void
NewSparseBitSet<A>::add_to_mask(BitSet b) {
  for (int i = 0; i <=limit; i++) {
    int offset = index[i];
    mask.o(b, offset);
  }
}

template<class A>
forceinline void
NewSparseBitSet<A>::intersect_with_mask() {
  for (int i = limit; i >= 0; i--) {
    int offset = index[i];
    Gecode::Support::BitSetData w = BitSet::a(words, mask, offset);
    if (!words.same(w, offset)) {
      words.setword(w, offset);
      if (w.none()) {
        index[i] = index[limit];
        limit--;
      }
    }
  }
}

template<class A>
forceinline int
NewSparseBitSet<A>::intersect_index(BitSet b) {
  for (int i = 0; i <= limit; i++) {
    int offset = index[i];
    if (!BitSet::a(words, b, offset).none()) {
      return offset;
    }
  }
  return -1;
}

template<class A>
forceinline void
NewSparseBitSet<A>::print() {
  std::cout << "words: ";
  words.print();
  std::cout << "limit: " << limit << std::cout;
  std::cout << "index: ";
  for (int i = 0; i < sz; i++) {
    std::cout << index[i] << " ";
  }
  std::cout << std::endl;
}

// template<class A>
// forceinline
// NewSparseBitSet<A>::~NewSparseBitSet(void) {
//   Gecode::Support::dispose(a);
// }

/**
 * Bit set
 */
// template<class A>
// forceinline
// BitSet::BitSet(A& a0, unsigned int s, bool set)
//   : BitSetBase(a0,s,set), a(a0) {}
// /// Copy bit set \a bs with allocator \a a
// template<class A>
// forceinline
// BitSet::BitSet(A& a0, const BitSet& bs)
//   : BitSetBase(a0,bs), a(a0) {}

template<class A>
forceinline
BitSet::BitSet(A& a, unsigned int sz, bool setbits)
  : BitSetBase(a,sz,setbits) {}


forceinline
BitSet::BitSet(const BitSet& bs) {
  Gecode::Support::RawBitSetBase::copy(bs.sz,bs);
}

forceinline BitSet&
BitSet::operator =(const BitSet& bs) {
  Gecode::Support::RawBitSetBase::copy(bs.sz,bs);
  return *this;
}

forceinline void
BitSet::o(BitSet a, unsigned int i) {
  assert(sz == a.sz);
  data[i].o(a.data[i]);
}

forceinline void
BitSet::a(BitSet a, unsigned int i) {
  assert(sz == a.sz);
  data[i].a(a.data[i]);
}

forceinline Gecode::Support::BitSetData
BitSet::o(BitSet a, BitSet b, unsigned int i) {
  assert(a.sz == b.sz && i < a.sz);
  Gecode::Support::BitSetData ab =
    Gecode::Support::BitSetData::o(a.data[i], b.data[i]);
  return ab;
}

forceinline Gecode::Support::BitSetData
BitSet::a(BitSet a, BitSet b, unsigned int i) {
  assert(a.sz == b.sz && i < a.sz);
  Gecode::Support::BitSetData ab =
    Gecode::Support::BitSetData::a(a.data[i], b.data[i]);
  return ab;
}

forceinline void
BitSet::clearword(unsigned int i, bool setbits) {
  assert(i < sz);
  data[i].init(setbits);
}

forceinline void
BitSet::setword(Gecode::Support::BitSetData a, unsigned int i) {
  assert(i < sz);
  data[i] = a;
}

forceinline Gecode::Support::BitSetData
BitSet::getword(unsigned int i) {
  assert(i < sz);
  return data[i];
}

forceinline int
BitSet::get_bpb() {
  return bpb;
}

forceinline bool
BitSet::same(Gecode::Support::BitSetData d, unsigned int i) {
  assert(false);
  return true;
  //return data[i].same(d);
}

forceinline void
BitSet::print() {
  for (int i = 0; i < sz; i++) {
    std::cout << get(i) + " ";
  }
  std::cout << std::endl;
}
