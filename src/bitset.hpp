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
  BitSet(A& a, unsigned int sz, bool setbits=false);
  /// Copy constructor
  BitSet(const BitSet&);
  /// Copy from bitset \a bs with allocator \a a
  template<class A>
  BitSet(A& a, const BitSet& bs);
  /// Copy from bitset \a bs with allocator \a a
  template<class A>
  BitSet(A& a, unsigned int sz, const BitSet& bs);
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
  void clearword(unsigned int i, bool setbits=false);
  /// Set a word \a i to \a a
  void setword(Gecode::Support::BitSetData a, unsigned int i);
  /// Get word \a i
  Gecode::Support::BitSetData getword(unsigned int i);
  /// Check if bit set has \a d on index \a i
  bool same(Gecode::Support::BitSetData d, unsigned int i);
  /// Get number of bits per base
  static int get_bpb();
  /// Copy \a sz bits from \a bs
  void copy(unsigned int sz, const BitSet& bs);
  /// Print bit set
  void print() const;
  /// Get size
  unsigned int size() const;
  /// Get number of set bits
  unsigned int nset() const;
  /// Get number of non-zero words
  unsigned int nset_words() const;
  /// Check if none bits are set (not counting the bit at position sz)
  //bool none() const;
  
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
  /// Default constructor (yields empty set)
  NewSparseBitSet(A& a);
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
  /// Clear \a set bits
  void clearall(unsigned int sz, bool setbits);
  /// Print bit set
  void print() const;
  /// Initialise sparse bit-set with space for \a s bits (only after call to default constructor)
  void init(unsigned int s);
  /// Get number of set bits
  unsigned int nset() const;
  /// Get limit
  unsigned int get_limit() const;
  /// Check if none bit is set in words
  bool none() const;
  /// Print mask
  void print_mask() const;
};

/**
 * Sparse bit set
 */
template<class A>
forceinline
NewSparseBitSet<A>::NewSparseBitSet(A& a0)
  : a(a0) {}

template<class A>
forceinline void
NewSparseBitSet<A>::init(unsigned int s) {
  words = BitSet(a,s,false);
  mask = BitSet(a,s,false);
  sz = s;
  int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
  limit = nwords - 1;
  index = a.template alloc<unsigned int>(nwords);
  for (int i = 0; i <= limit; i++) {
    index[i] = i;
  }
 }

template<class A>
forceinline
NewSparseBitSet<A>::NewSparseBitSet(A& a0, unsigned int s)
  : a(a0), words(a,s,false), mask(a,s,false), sz(s) {
  // Calculate number of required words
  int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
  //std::cout << "s: " << s << ", nwords: " << nwords << std::endl;
  limit = nwords - 1;
  index = a.template alloc<unsigned int>(nwords);
  for (int i = 0; i <= limit; i++) {
    index[i] = i;
  }
  //std::cout << "after sparse bit set consturctor: " << std::endl;
  //print();
 
  
}

template<class A>
forceinline
NewSparseBitSet<A>::NewSparseBitSet(A& a0, const NewSparseBitSet<A>& sbs)
  : a(a0), sz(sbs.sz), limit(sbs.limit), words(a,sbs.words), mask(a,sbs.sz,false) {
  //std::cout << "copy sparsebitset" << std::endl;

  index = a.template alloc<unsigned int>(limit + 1);
  for (int i = 0; i <= limit; i++) {
    index[i] = sbs.index[i];
  }
  // std::cout << "copied: " << std::endl;
  // sbs.print();
  // std::cout << "copy: " << std::endl;
  // print();
  // std::cout << "end copy sparsebitset" << std::endl;
}

template<class A>
forceinline bool
NewSparseBitSet<A>::is_empty() const {
  //std::cout << "empty!" << std::endl;
  if (limit == -1) {
    assert(words.none());
  }
  if (words.none() && limit != -1) {
    words.print();
    assert(limit == -1);
  }
  return limit == -1;
}

template<class A>
forceinline void
NewSparseBitSet<A>::clear_mask() {
  for (int i = 0; i <= limit; i++) {
    int offset = index[i];
    mask.clearword(offset, false);
    if (!mask.getword(offset).none()) {
      std::cout << "word " << i << std::endl;
      mask.print();
    }
    assert(mask.getword(offset).none());
  }
  mask.clearall(false);
  if (!mask.none())
    mask.print();
  assert(mask.none());
}

template<class A>
forceinline void
NewSparseBitSet<A>::add_to_mask(BitSet b) {
  //std::cout << "add_to_mask" << std::endl;
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
    // std::cout << "and of ";
    // words.print();
    // std::cout << " + ";
    // mask.print();
    // std::cout << " gives none =" << w.none() << std::endl;
    
    if (!words.same(w, offset)) {
      words.setword(w, offset);
      if (w.none()) {
        index[i] = index[limit];
        limit--;
        //std::cout << words.nset() << " " << words.nset_words() << " " << limit << std::endl;
      }
      if (words.none() && limit != -1) {
        words.print();
        std::cout << "w.none() = "<< w.none() << std::endl;
        std::cout << "limit = " << limit << std::endl;
        assert(false);
      }
    }
  }
  if (words.none() && limit != -1) {
    words.print();
    assert(false);
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
NewSparseBitSet<A>::clearall(unsigned int sz, bool setbits) {
  int start_bit = 0;
  int complete_words = sz / BitSet::get_bpb();
  if (complete_words > 0) {
    start_bit = complete_words * BitSet::get_bpb() + 1;
    words.Gecode::Support::RawBitSetBase::clearall(start_bit - 1,setbits);
  }
  for (int i = start_bit; i < sz; i++) {
    setbits ? words.set(i) : words.clear(i);
  }

}

template<class A>
forceinline void
NewSparseBitSet<A>::print() const {
  std::cout << "words: ";
  words.print();
  std::cout << "mask: ";
  mask.print();

  std::cout << "index: ";
  int nwords = sz != 0 ? (sz - 1) / words.get_bpb() + 1 : 0;
  for (int i = 0; i <= limit; i++) {
    std::cout << index[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "limit: " << limit << std::endl;
  //std::cout << "words.none() = " << words.none() << std::endl;
}

template<class A>
forceinline unsigned int
NewSparseBitSet<A>::nset() const {
  return words.nset();
}

template<class A>
forceinline unsigned int
NewSparseBitSet<A>::get_limit() const {
  return limit;
}

template<class A>
forceinline bool
NewSparseBitSet<A>::none() const {
  return words.none();
}

template<class A>
forceinline void
NewSparseBitSet<A>::print_mask() const {
  mask.print();
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
  : BitSetBase(a,sz,setbits) {
  Gecode::Support::RawBitSetBase::clear(sz);
}

// TODO
forceinline bool
BitSet::same(Gecode::Support::BitSetData d, unsigned int i) {
  return false;
  //return data[i].same(d);
}

forceinline
BitSet::BitSet(const BitSet& bs) {
  //std::cout << "copy BitSet" << std::endl;
  sz = bs.sz;
  data = bs.data;
  for (int i = 0; i < sz / bpb; i++) {
    data[i] = bs.data[i];
  }
  Gecode::Support::RawBitSetBase::clear(sz);
  //Gecode::Support::RawBitSetBase::copy(bs.sz,bs);
  //std::cout << "end copy BitSet" << std::endl;
}

template<class A>
forceinline
BitSet::BitSet(A& a, const BitSet& bs)
  : BitSetBase(a,bs) {
  Gecode::Support::RawBitSetBase::clear(sz);
}

template<class A>
forceinline
BitSet::BitSet(A& a, unsigned int sz, const BitSet& bs)
  : BitSetBase(a,sz) {
  for (unsigned int i = Gecode::Support::BitSetData::data(sz+1); i--; )
    data[i] = bs.data[i];
  // Set a bit at position sz as sentinel (for efficient next)
  //set(sz);
}


forceinline BitSet&
BitSet::operator =(const BitSet& bs) {
  //std::cout << "assignement operator" << std::endl;
  //Gecode::Support::RawBitSetBase::copy(bs.sz,bs);
  sz = bs.sz;
  data = bs.data;
  for (int i = 0; i < sz / bpb; i++) {
    data[i] = bs.data[i];
  }
  return *this;
}

forceinline void
BitSet::copy(unsigned int sz, const BitSet& bs) {
  Gecode::Support::RawBitSetBase::copy(sz,bs);
}

forceinline
BitSet::BitSet(void) {}

forceinline void
BitSet::o(BitSet a, unsigned int i) {
  if (sz != a.sz) {
    std::cout << "sz=" << sz << ", a.sz=" << a.sz << std::endl;
  }
  assert(sz == a.sz);
  assert(i < sz);
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
  if (setbits) {
    assert(data[i].all());    
  } else {
    assert(data[i].none());
  }
}

forceinline void
BitSet::setword(Gecode::Support::BitSetData a, unsigned int i) {
  assert(i < sz);
  data[i] = a;
}

forceinline Gecode::Support::BitSetData
BitSet::getword(unsigned int i) {
  assert(i < sz);
  //std::cout << data[i].none() << std::endl;
  return data[i];
}

forceinline int
BitSet::get_bpb() {
  return bpb;
}

forceinline void
BitSet::print() const {
  for (int i = 0; i < sz; i++) {
    std::cout << get(i) << " ";
  }
  std::cout << std::endl;
}

forceinline unsigned int
BitSet::size() const {
  return sz;
}

forceinline unsigned int
BitSet::nset() const {
  int count = 0;
  for (int i = 0; i < sz; i++) {
    if (get(i)) {
      count++;
    }
  }
  return count;
}

forceinline unsigned int
BitSet::nset_words() const {
  int count = 0;
  bool verbose = false;
  if (none()) {
    verbose = true;
  }
  for (unsigned int i = Gecode::Support::BitSetData::data(sz+1); i--; ) {
    count += !data[i].none();
    if (verbose) {
      std::cout << "i=" << i << std::endl;
    }
    if (verbose && !data[i].none()) {
      std::cout << "word " << i << "is not none " << std::endl;
      std::cout << "get(sz)" << RawBitSetBase::get(sz) << std::endl;
      std::cout << "nwords (?) = " << Gecode::Support::BitSetData::data(sz+1) << std::endl;
      std::cout << "nbits: " << sz << std::endl; 
      print();
      assert(false);
    }
  }
  return count;
}

// forceinline bool
// BitSet::none() const {
  
//   for (unsigned int i = Gecode::Support::BitSetData::data(sz+1); i--; ) {
//     count += !data[i].none();

// }
