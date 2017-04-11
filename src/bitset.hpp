class BitSet : public Gecode::Support::BitSetBase {
public:
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
  /// Allocate for \a sz bits and allocator \a a (only after default constructor)
  template<class A>
  void allocate(A& a, unsigned int sz);
  /// Perform "or" with \a a of word index \a i
  void o(const BitSet& a, unsigned int i);
  /// Perform "and" with \a a of word index \a 
  void a(const BitSet& a, unsigned int i);
  /// Return "or" of \a a and \a b of word index \a i
  static Gecode::Support::BitSetData o(const BitSet& a, const BitSet& b,
                                       unsigned int i);
  /// Return "and" of \a a and \a b of word index \a i
  static Gecode::Support::BitSetData a(const BitSet& a, const BitSet& b,
                                       unsigned int i);
  /// Clear all bits at word index \a i
  void clearword(unsigned int i, bool setbits=false);
  /// Set a word \a i to \a a
  void setword(Gecode::Support::BitSetData a, unsigned int i);
  /// Get word \a i
  Gecode::Support::BitSetData getword(unsigned int i) const;
  /// Check if bit set has \a d on word index \a i
  bool same(Gecode::Support::BitSetData d, unsigned int i) const;
  /// Get number of bits per base
  static int get_bpb();
  /// Copy \a sz bits from \a bs
  void copy(unsigned int sz, const BitSet& bs);
  /// Dispose memory for bit set
  template<class A>
  void dispose(A& a);
  /// Next (disabled)
  unsigned int next(unsigned int i) const;
  /// Return number of set bits among the bits 0 to \a i
  unsigned int nset(unsigned int i) const;
  /** Debugging **/
  /// Print bit set
  void print() const;
  /// Get size
  unsigned int size() const;
  /// Get number of set bits
  unsigned int nset() const;
  /// Get number of non-zero words
  unsigned int nset_words() const;
};

template<class A>
class SparseBitSet {
private:
  /// Allocator
  A& al;
  /// Words (FIXME: why public?)
  BitSet words;
  /// Index
  unsigned int* index;
  /// Limit
  int limit;
  /// Size
  int sz;
public:
  /// Default constructor (yields empty set)
  SparseBitSet(A& a);
  /// Copy sparse bit set \a sbs with allocator \a a
  SparseBitSet(A& a, const SparseBitSet& sbs);
  /// Destructor
  ~SparseBitSet(void);
  /// Initialise sparse bit-set with space for \a s bits (only after call to default constructor)
  void init(unsigned int s);
  /// Check if sparse bit set is empty
  bool is_empty() const;
  /// Clear the mask
  void clear_mask(BitSet& mask);
  /// Add bits in \a b to \a mask
  void add_to_mask(const BitSet& b, BitSet& mask) const;
  /// Intersect words with \a mask
  bool intersect_with_mask(const BitSet& mask);
  /// Get the index of a non-zero intersect with \a b, or -1 if none exists
  int intersect_index(const BitSet& b) const;
  /// Perform "and" with words and \a b at word index \a i
  Gecode::Support::BitSetData a(const BitSet& b, unsigned int i);
  /// Get the number of bits
  unsigned int size() const;
  
  /** Debugging purpose **/
  /// Print bit set
  void print() const;
  /// Get number of set bits
  unsigned int nset() const;
  /// Get limit
  unsigned int get_limit() const;
  /// Check if none bit is set in words
  bool none() const;
  /// Print mask
  void print_mask() const;
  /// Return "and" of words at index \a a and \a b
  
private: 
  /// Clear \a set bits in words
  void clearall(unsigned int sz, bool setbits);
};

/**
 * Bit set
 */
forceinline
BitSet::BitSet(void) {}

template<class A>
forceinline
BitSet::BitSet(A& a,unsigned int sz,bool setbits)
  : BitSetBase(a,sz,setbits) {
  // Clear bit sz (set in RawBitSetBase)
  Gecode::Support::RawBitSetBase::clear(sz);
}

forceinline
BitSet::BitSet(const BitSet& bs) {
  sz = bs.sz;
  data = bs.data;
  // Clear bit sz
  Gecode::Support::RawBitSetBase::clear(sz);
}

template<class A>
forceinline
BitSet::BitSet(A& a, const BitSet& bs)
  : BitSetBase(a,bs) {
  // Clear bit sz
  Gecode::Support::RawBitSetBase::clear(sz);
}

template<class A>
forceinline
BitSet::BitSet(A& a, unsigned int sz, const BitSet& bs)
  : BitSetBase(a,sz) {
  for (unsigned int i = Gecode::Support::BitSetData::data(sz+1); i--; )
    data[i] = bs.data[i];
  // Clear bit sz
  Gecode::Support::RawBitSetBase::clear(sz);
}

forceinline BitSet&
BitSet::operator =(const BitSet& bs) {
  sz = bs.sz;
  data = bs.data;
  // Clear bit sz
  Gecode::Support::RawBitSetBase::clear(sz);
  return *this;
}

template<class A>
forceinline void
BitSet::allocate(A& a, unsigned int sz0) {
  RawBitSetBase::allocate(a,sz0);
  sz = sz0;
}


forceinline void
BitSet::copy(unsigned int sz, const BitSet& bs) {
  Gecode::Support::RawBitSetBase::copy(sz,bs);
}

forceinline bool
BitSet::same(Gecode::Support::BitSetData d, unsigned int i) const {
  return data[i].same(d);
}

template<class A>
forceinline void
BitSet::dispose(A& a) {
  RawBitSetBase::dispose(a,sz);
}

forceinline void
BitSet::o(const BitSet& a, unsigned int i) {
  assert(i < sz && i < a.sz);
  data[i].o(a.data[i]);
}

forceinline void
BitSet::a(const BitSet& a, unsigned int i) {
  assert(i < sz && i < a.sz);
  data[i].a(a.data[i]);
}

forceinline Gecode::Support::BitSetData
BitSet::o(const BitSet& a, const BitSet& b, unsigned int i) {
  assert(i < a.sz && i < b.sz);
  return Gecode::Support::BitSetData::o(a.data[i], b.data[i]);
}

forceinline Gecode::Support::BitSetData
BitSet::a(const BitSet& a, const BitSet& b, unsigned int i) {
  if (i >= a.sz || i >= b.sz) {
    printf("i = %d, a.sz = %d, b.sz = %d\n",i,a.sz,b.sz);
  }
  assert(i < a.sz && i < b.sz);
  return Gecode::Support::BitSetData::a(a.data[i], b.data[i]);;
}

forceinline void
BitSet::clearword(unsigned int i, bool setbits) {
  assert(i < sz);
  data[i].init(setbits);
#ifdef DEBUG
  if (setbits) {
    assert(data[i].all());    
  } else {
    assert(data[i].none());
  }
#endif // DEBUG
}

forceinline void
BitSet::setword(Gecode::Support::BitSetData a, unsigned int i) {
  assert(i < sz);
  data[i] = a;
}

forceinline Gecode::Support::BitSetData
BitSet::getword(unsigned int i) const {
  assert(i < sz);
  return data[i];
}

forceinline int
BitSet::get_bpb() {
  return bpb;
}

forceinline unsigned int
BitSet::next(unsigned int) const {
  GECODE_NEVER;
  return -1;
}

/** Debugging purpose **/

forceinline void
BitSet::print() const {
  for (unsigned int i = 0; i < sz; i++) {
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
  for (unsigned int i = 0; i < sz; i++) {
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

/**
 * Sparse bit set
 */
template<class A>
forceinline
SparseBitSet<A>::SparseBitSet(A& a0)
  : al(a0) {}

template<class A>
forceinline void
SparseBitSet<A>::init(unsigned int s) {
  words = BitSet(al,s,false);
  sz = s;
  int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
  limit = nwords - 1;
  index = al.template alloc<unsigned int>(nwords);
  for (int i = limit+1; i--; )
    index[i] = i;
  // Set the set nr of bits in words
  clearall(s, true);
 }

template<class A>
forceinline
SparseBitSet<A>::SparseBitSet(A& a0, const SparseBitSet<A>& sbs)
  : al(a0), words(al,sbs.words),
    limit(sbs.limit), sz(sbs.sz)  {
  // Copy limit nr of elements in index
  index = al.template alloc<unsigned int>(limit + 1);
  for (int i = limit+1; i--; )
    index[i] = sbs.index[i];
}

template<class A>
forceinline bool
SparseBitSet<A>::is_empty() const {
  return limit == -1;
}

template<class A>
forceinline void
SparseBitSet<A>::clear_mask(BitSet& mask) {
  for (int i = 0; i <= limit; i++) {
    int offset = index[i];
    mask.clearword(offset, false);
  }
}

template<class A>
forceinline void
SparseBitSet<A>::add_to_mask(const BitSet& b, BitSet& mask) const {
  for (int i = 0; i<=limit; i++) {
    int offset = index[i];
    mask.o(b,offset);
  }
}

template<class A>
forceinline bool
SparseBitSet<A>::intersect_with_mask(const BitSet& mask) {
  bool diff = false;
  for (int i = limit; i >= 0; i--) {
    int offset = index[i];
    Gecode::Support::BitSetData w = a(mask, offset);
    if (!words.same(w, offset)) {
      diff = true;
      words.setword(w, offset);
      if (w.none()) {
        index[i] = index[limit];
        limit--;
      }
    }
  }
  return diff;
}

template<class A>
forceinline int
SparseBitSet<A>::intersect_index(const BitSet& b) const {
  for (int i = 0; i <= limit; i++) {
    int offset = index[i];
    if (!BitSet::a(words,b,offset).none())
      return offset;
  }
  return -1;
}

template<class A>
forceinline void
SparseBitSet<A>::clearall(unsigned int sz, bool setbits) {
  int start_bit = 0;
  int complete_words = sz / BitSet::get_bpb();
  if (complete_words > 0) {
    start_bit = complete_words * BitSet::get_bpb() + 1;
    words.Gecode::Support::RawBitSetBase::clearall(start_bit - 1,setbits);
  }
  for (unsigned int i = start_bit; i < sz; i++) {
    setbits ? words.set(i) : words.clear(i);
  }
}

template<class A>
forceinline Gecode::Support::BitSetData
SparseBitSet<A>::a(const BitSet& b, unsigned int i) {
  return BitSet::a(words, b, i);
}

template<class A>
forceinline unsigned int
SparseBitSet<A>::size() const {
  return words.size();
}

/** Debugging purpose **/

template<class A>
forceinline void
SparseBitSet<A>::print() const {
  std::cout << "words: ";
  words.print();
  // std::cout << "mask: ";
  // mask.print();
  std::cout << "index: ";
  //int nwords = sz != 0 ? (sz - 1) / words.get_bpb() + 1 : 0;
  for (int i = 0; i <= limit; i++) {
    std::cout << index[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "limit: " << limit << std::endl;
  //std::cout << "words.none() = " << words.none() << std::endl;
}

template<class A>
forceinline unsigned int
SparseBitSet<A>::nset() const {
  return words.nset();
}

template<class A>
forceinline unsigned int
SparseBitSet<A>::get_limit() const {
  return limit;
}

template<class A>
forceinline bool
SparseBitSet<A>::none() const {
  return words.none();
}

template<class A>
forceinline
SparseBitSet<A>::~SparseBitSet(void) {
  words.BitSet::dispose(al);
  al.template free<unsigned int>(index,limit+1);
}
