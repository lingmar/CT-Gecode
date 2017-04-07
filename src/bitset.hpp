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
  /// Check if bit set has \a d on word index \a i
  bool same(Gecode::Support::BitSetData d, unsigned int i);
  /// Get number of bits per base
  static int get_bpb();
  /// Copy \a sz bits from \a bs
  void copy(unsigned int sz, const BitSet& bs);
  /// Next (disabled)
  unsigned int next(unsigned int i) const;
  // /// Init bit set at memory address \a dest
  // void init(Gecode::Support::BitSetData* dest, unsigned int sz, bool setbits=false);
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
  /// Mask
  BitSet mask;
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
  //~SparseBitSet(void);
  /// Initialise sparse bit-set with space for \a s bits (only after call to default constructor)
  void init(unsigned int s, unsigned int set);
  /// Check if sparse bit set is empty
  bool is_empty() const;
  /// Clear the mask
  void clear_mask();
  /// Add bits in \a b to mask
  void add_to_mask(BitSet b);
  /// Intersect words with mask
  bool intersect_with_mask();
  /// Get the index of a non-zero intersect with \a b, or -1 if none exists
  int intersect_index(BitSet b);
  Gecode::Support::BitSetData a(BitSet b, unsigned int i);

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

// forceinline void
// BitSet::init(Gecode::Support::BitSetData* dest,
//              unsigned int s, bool setbits) {
//   assert(data == NULL);
//   sz = s;
//   data=dest;
//   for (unsigned int i=Gecode::Support::BitSetData::data(sz+1); i--; )
//     data[i].init(setbits);
// }

forceinline BitSet&
BitSet::operator =(const BitSet& bs) {
  sz = bs.sz;
  data = bs.data;
  // Clear bit sz
  Gecode::Support::RawBitSetBase::clear(sz);
  return *this;
}

forceinline void
BitSet::copy(unsigned int sz, const BitSet& bs) {
  Gecode::Support::RawBitSetBase::copy(sz,bs);
}

forceinline bool
BitSet::same(Gecode::Support::BitSetData d, unsigned int i) {
  return data[i].same(d);
}

forceinline void
BitSet::o(BitSet a, unsigned int i) {
  assert(i < sz && i < a.sz);
  data[i].o(a.data[i]);
}

forceinline void
BitSet::a(BitSet a, unsigned int i) {
  assert(i < sz && i < a.sz);
  data[i].a(a.data[i]);
}

forceinline Gecode::Support::BitSetData
BitSet::o(BitSet a, BitSet b, unsigned int i) {
  assert(i < a.sz && i < b.sz);
  return Gecode::Support::BitSetData::o(a.data[i], b.data[i]);
}

forceinline Gecode::Support::BitSetData
BitSet::a(BitSet a, BitSet b, unsigned int i) {
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
BitSet::getword(unsigned int i) {
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

forceinline unsigned int
BitSet::nset(unsigned int i) const {
  assert(i < sz);
  unsigned int count = 0;
  static unsigned int nbases = Gecode::Support::BitSetData::data(sz+1);
  // Number of spare bits
  static unsigned int r = i % nbases;
  // Create mask
  // BitSet with size sz, 0-r bits set
  Gecode::Support::BitSetData mask;
  mask.init(false); // 00000000
  Gecode::Support::BitSetData aux;
  aux.init(true);   // 11111111
  mask.Gecode::Support::BitSetData::o(aux,r);    // 11111000

  // Perform "and" with the mask
  Gecode::Support::BitSetData masked =
    Gecode::Support::BitSetData::a(data[nbases-1], mask);

  // Count set bits
  
  return count;
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
SparseBitSet<A>::init(unsigned int s, unsigned int set) {
  //std::cout << "INIT" << std::endl;
  words = BitSet(al,s,false);
  mask = BitSet(al,s,false);
  sz = s;
  int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
  limit = nwords - 1;
  index = al.template alloc<unsigned int>(nwords);
  for (int i = 0; i <= limit; i++) {
    index[i] = i;
  }
  // Set set nr of bits
  clearall(set, true);
 }

template<class A>
forceinline
SparseBitSet<A>::SparseBitSet(A& a0, const SparseBitSet<A>& sbs)
  : al(a0), words(al,sbs.words), mask(al,sbs.sz,false), limit(sbs.limit), sz(sbs.sz)  {
  // Only copy limit nr of elements in index
  index = al.template alloc<unsigned int>(limit + 1);
  for (int i = 0; i <= limit; i++) {
    index[i] = sbs.index[i];
  }
}

template<class A>
forceinline bool
SparseBitSet<A>::is_empty() const {
#ifdef DEBUG
  if (limit == -1) {
    assert(words.none());
  }
  if (words.none() && limit != -1) {
    words.print();
    assert(limit == -1);
  }
#endif // DEBUG
  return limit == -1;
}

template<class A>
forceinline void
SparseBitSet<A>::clear_mask() {
  for (int i = 0; i <= limit; i++) {
    int offset = index[i];
    mask.clearword(offset, false);
#ifdef DEBUG
    assert(mask.getword(offset).none());
#endif // DEBUG
  }
}

template<class A>
forceinline void
SparseBitSet<A>::add_to_mask(BitSet b) {
  for (int i = 0; i<=limit; i++) {
    int offset = index[i];
    mask.o(b,offset);
  }
}

template<class A>
forceinline bool
SparseBitSet<A>::intersect_with_mask() {
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
SparseBitSet<A>::intersect_index(BitSet b) {
  for (int i = 0; i <= limit; i++) {
    int offset = index[i];
    if (!a(b,offset).none()) {
      return offset;
    }
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
SparseBitSet<A>::a(BitSet b, unsigned int i) {
  return BitSet::a(words, b, i);
}

/** Debugging purpose **/

template<class A>
forceinline void
SparseBitSet<A>::print() const {
  std::cout << "words: ";
  words.print();
  std::cout << "mask: ";
  mask.print();
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

// template<class A>
// forceinline
// SparseBitSet<A>::~SparseBitSet(void) {
//   Gecode::Support::dispose(a);
// }
