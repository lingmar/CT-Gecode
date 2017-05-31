//#define forceinline __attribute__ ((noinline))
using namespace Gecode;

class BitSet : public Gecode::Support::BitSetBase {
private:
  /// Copy constructor (disabled)
  BitSet(const BitSet&);
public:
  /// Default constructor (yields empty set)
  BitSet(void);
  /// Initialize for \a sz bits and allocator \a a
  template<class A>
  BitSet(A& a, unsigned int sz, bool setbits=false);
  /// Copy from bitset \a bs with allocator \a a
  template<class A>
  BitSet(A& a, const BitSet& bs);
  /// Copy from bitset \a bs with allocator \a a
  template<class A>
  BitSet(A& a, unsigned int sz, const BitSet& bs);
  /// Initialise for \a sz bits with allocator \a a
  template<class A>
  void init(A& a, unsigned int sz, bool setbits=false);
  /// Allocate for \a sz bits and allocator \a a (only after default constructor)
  template<class A>
  void allocate(A& a, unsigned int sz);
  /// Empty set
  bool empty() const;
  /// Perform "or" with \a a of word index \a i
  void o(const BitSet& a, unsigned int i);
  /// Perform "and" with \a a of word index \a 
  void a(const BitSet& a, unsigned int i);
  /// Perform "and" with ith word of a and jth word of b
  static Gecode::Support::BitSetData a(const BitSet& a, unsigned int i,
                                       const BitSet& b, unsigned int j);
  /// Return "or" of \a a and \a b of word index \a i
  static Gecode::Support::BitSetData o(const BitSet& a, const BitSet& b,
                                       unsigned int i);
  /// Return "and" of \a a and \a b of word index \a i
  static Gecode::Support::BitSetData a(const BitSet& a, const BitSet& b,
                                       unsigned int i);
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
  /// Test whether exactly one bit is set for word index \i
  bool one(unsigned int i) const;
  /// Perform or with \a a and \a b with mapping \a map
  static void or_by_map(BitSet& a, const BitSet& b,
                        const int* map, unsigned int map_last);
  /// TBA
  static void init_from_bs(BitSet& m, const BitSet& a, const BitSet& b,
                           const int* map, unsigned int map_last);
  /// Find index of non-empty intersecting word with \a a and \a b
  static int intersect_index_by_map(const BitSet& a, const BitSet& b,
                             const int* map, unsigned int map_last);
  /// Intersect \a with mask \a b with words described by \a map
  static void intersect_by_map(BitSet& a, const BitSet& b,
                               int* map, int* map_last);
  /// Intersect \a with mask \a b with words described by \a map
  static void intersect_by_map_sparse(BitSet& a, const BitSet& b,
                                      int* map, int* map_last);
  /// Intersect \a with mask \a b with words described by \a map
  static void intersect_by_map_sparse_two(BitSet& a, const BitSet& m1,
                                          const BitSet& m2, int* map,
                                          int* map_last);
  /// Nand \a with mask \b with words described by \a map
  static void nand_by_map(BitSet& a, const BitSet& b,
                             int* map, int* map_last);
  /// Clear the words in \a b that occur in \a map
  static void clear_to_limit(BitSet& b, unsigned int limit);
  /// Clear the words in \a b that occur in \a map
  static void flip_by_map(BitSet& b, const int* map, unsigned int map_last);
  
  /** Debugging **/
  /// Print bit set
  void print() const;
  /// Get size
  unsigned int size() const;
};

/**
 * Bit set
 */
forceinline
BitSet::BitSet(void) {
  sz = 0;
}

template<class A>
forceinline
BitSet::BitSet(A& a,unsigned int sz,bool setbits)
  : BitSetBase(a,sz,setbits) {
  // Clear bit sz (set in RawBitSetBase)
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
  assert(sz <= bs.sz);
  for (unsigned int i = Gecode::Support::BitSetData::data(sz+1); i--; )
    data[i] = bs.data[i];
  // Clear bit sz
  Gecode::Support::RawBitSetBase::clear(sz);
}

template<class A>
forceinline void
BitSet::allocate(A& a, unsigned int sz0) {
  RawBitSetBase::allocate(a,sz0);
  sz = sz0;
}

forceinline bool
BitSet::empty() const {
  return sz == 0;
}

forceinline void
BitSet::copy(unsigned int sz0, const BitSet& bs) {
  //  assert(sz0 <= bs.sz);
  Gecode::Support::RawBitSetBase::copy(sz0,bs);
}

forceinline bool
BitSet::same(Gecode::Support::BitSetData d, unsigned int i) const {
  assert(i < Support::BitSetData::data(sz));
  return data[i].Gecode::Support::BitSetData::same(d);
}

template<class A>
forceinline void
BitSet::dispose(A& a) {
  RawBitSetBase::dispose(a,sz);
}

forceinline void
BitSet::o(const BitSet& a, unsigned int i) {
  assert(i < Support::BitSetData::data(sz));
  assert(i < Support::BitSetData::data(a.sz));
  data[i].o(a.data[i]);
}

forceinline void
BitSet::a(const BitSet& a, unsigned int i) {
  assert(i < Support::BitSetData::data(sz));
  assert(i < Support::BitSetData::data(a.sz));
  data[i].a(a.data[i]);
}

forceinline Gecode::Support::BitSetData
BitSet::o(const BitSet& a, const BitSet& b, unsigned int i) {
  assert(i < Support::BitSetData::data(a.sz));
  assert(i < Support::BitSetData::data(b.sz));  
  return Gecode::Support::BitSetData::o(a.data[i], b.data[i]);
}

forceinline Gecode::Support::BitSetData
BitSet::a(const BitSet& a, const BitSet& b, unsigned int i) {
  assert(i < Support::BitSetData::data(a.sz));
  assert(i < Support::BitSetData::data(b.sz));  
  return Gecode::Support::BitSetData::a(a.data[i], b.data[i]);;
}

forceinline Gecode::Support::BitSetData
BitSet::a(const BitSet& a, unsigned int i,
          const BitSet& b, unsigned int j) {
  assert(i < Support::BitSetData::data(a.sz));
  assert(j < Support::BitSetData::data(b.sz));  
  return Gecode::Support::BitSetData::a(a.data[i], b.data[j]);;
}


forceinline Gecode::Support::BitSetData
BitSet::getword(unsigned int i) const {
  assert(i < Support::BitSetData::data(sz));
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

template<class A>
forceinline void
BitSet::init(A& a, unsigned int s, bool setbits) {
  assert(sz == 0);
  RawBitSetBase::init(a,s,setbits);
  sz=s;
  // Clear sentinel bit
  Gecode::Support::RawBitSetBase::clear(sz);
}

forceinline bool
BitSet::one(unsigned int i) const {
  assert(i < Support::BitSetData::data(sz));
  return data[i].Gecode::Support::BitSetData::one();
}

forceinline void
BitSet::init_from_bs(BitSet& m, const BitSet& a, const BitSet& b,
                     const int* map, unsigned int map_last) {
  Gecode::Support::BitSetData* a_data = a.data;
  Gecode::Support::BitSetData* b_data = b.data;
  Gecode::Support::BitSetData* m_data = m.data;
  assert(map_last < Support::BitSetData::data(m.sz));
  for (int i = 0; i <= map_last; i++) {
    int offset = map[i];
    assert(offset < Support::BitSetData::data(a.sz));
    assert(offset < Support::BitSetData::data(b.sz));
    m_data[i] = Support::BitSetData::o(a_data[offset],b_data[offset]);
  }
}


forceinline void
BitSet::or_by_map(BitSet& a, const BitSet& b,
                  const int* map, unsigned int map_last) {
  Gecode::Support::BitSetData* a_data = a.data;
  Gecode::Support::BitSetData* b_data = b.data;
  assert(map_last < Support::BitSetData::data(a.sz));
  for (int i = 0; i <= map_last; i++) {
    int offset = map[i];
    assert(offset < Support::BitSetData::data(b.sz));
    a_data[i].o(b_data[offset]);
  }
}

forceinline int
BitSet::intersect_index_by_map(const BitSet& a, const BitSet& b,
                        const int* map, unsigned int map_last) {
  using namespace Gecode::Support;
  BitSetData* a_data = a.data;
  BitSetData* b_data = b.data;
  assert(map_last < Support::BitSetData::data(a.sz));
  for (int i = map_last; i >= 0; i--) {
    int offset = map[i];
    assert(offset < Support::BitSetData::data(b.sz));
    if (!BitSetData::a(a_data[i],b_data[offset]).none())
      return i;
  }
  return -1;
}

forceinline void
BitSet::intersect_by_map(BitSet& a, const BitSet& b,
                         int* map, int* map_last) {
  using namespace Gecode::Support;
  BitSetData* a_data = a.data;
  BitSetData* b_data = b.data;
  assert(*map_last >= 0);
  assert(*map_last < Support::BitSetData::data(a.sz));
  assert(*map_last < Support::BitSetData::data(b.sz));
  int local_map_last = *map_last;
  assert(!a_data[local_map_last].none());
  for (int i = local_map_last; i >= 0; i--) {
    assert(!a_data[i].none());
    assert(!a_data[local_map_last].none());
    BitSetData w = BitSetData::a(a_data[i],b_data[i]);
    if (!w.same(a_data[i])) {
      a_data[i] = w;
      if (w.none()) {
        assert(a_data[i].none());
        assert(i == local_map_last || !a_data[local_map_last].none());
        a_data[i] = a_data[local_map_last];
        a_data[local_map_last] = w;
        map[i] = map[local_map_last];
        local_map_last--;
      }
    }
    assert(i == local_map_last + 1 || !a_data[i].none());
  }
  *map_last = local_map_last;
}

/// Intersect \a with mask \a b with words described by \a map
forceinline void
BitSet::intersect_by_map_sparse(BitSet& a, const BitSet& b,
                                    int* map, int* map_last) {
  using namespace Gecode::Support;
  BitSetData* a_data = a.data;
  BitSetData* b_data = b.data;
  assert(*map_last >= 0);
  assert(*map_last < Support::BitSetData::data(a.sz));
  int local_map_last = *map_last;
  for (int i = local_map_last; i >= 0; i--) {
    int offset = map[i];
    assert(offset < Support::BitSetData::data(b.sz));
    BitSetData w = BitSetData::a(a_data[i],b_data[offset]);
    if (!w.same(a_data[i])) {
      a_data[i] = w;
      if (w.none()) {
        assert(a_data[i].none());
        assert(i == local_map_last || !a_data[local_map_last].none());
        a_data[i] = a_data[local_map_last];
        a_data[local_map_last] = w;
        map[i] = map[local_map_last];
        local_map_last--;
      }
    }
    assert(i == local_map_last + 1 || !a_data[i].none());
  }
  *map_last = local_map_last;
}

/// Intersect \a with mask \a b with words described by \a map
forceinline void
BitSet::intersect_by_map_sparse_two(BitSet& a, const BitSet& b1,
                                    const BitSet& b2, int* map,
                                    int* map_last) {
  using namespace Gecode::Support;
  BitSetData* a_data = a.data;
  BitSetData* b1_data = b1.data;
  BitSetData* b2_data = b2.data;
  assert(*map_last >= 0);
  assert(*map_last < Support::BitSetData::data(a.sz));
  int local_map_last = *map_last;
  for (int i = local_map_last; i >= 0; i--) {
    int offset = map[i];
    assert(offset < Support::BitSetData::data(b1.sz));
    assert(offset < Support::BitSetData::data(b2.sz));
    BitSetData w = BitSetData::a(a_data[i],
                                 BitSetData::o(b1_data[offset],b2_data[offset]));
    if (!w.same(a_data[i])) {
      a_data[i] = w;
      if (w.none()) {
        assert(a_data[i].none());
        assert(i == local_map_last || !a_data[local_map_last].none());
        a_data[i] = a_data[local_map_last];
        a_data[local_map_last] = w;
        map[i] = map[local_map_last];
        local_map_last--;
      }
    }
    assert(i == local_map_last + 1 || !a_data[i].none());
  }
  *map_last = local_map_last;
}


forceinline void
BitSet::nand_by_map(BitSet& a, const BitSet& b,
                    int* map, int* map_last) {
  using namespace Gecode::Support;
  BitSetData* a_data = a.data;
  BitSetData* b_data = b.data;
  int local_map_last = *map_last;
  assert(local_map_last < Support::BitSetData::data(a.sz));
  for (int i = local_map_last; i >= 0; i--) {
    int offset = map[i];
    assert(offset < Support::BitSetData::data(b.sz));
    BitSetData flipped = BitSetData::reverse(b_data[offset]);
    BitSetData w = BitSetData::a(a_data[i],flipped);
    if (!w.same(a_data[i])) {
      a_data[i] = w;
      if (w.none()) {
        assert(a_data[i].none());
        assert(i == local_map_last || !a_data[local_map_last].none());
        a_data[i] = a_data[local_map_last];
        a_data[local_map_last] = w;
        map[i] = map[local_map_last];
        local_map_last--;
      }
    }
    assert(i == local_map_last + 1 || !a_data[i].none());
  }
  *map_last = local_map_last;
}

forceinline void
BitSet::clear_to_limit(BitSet& b, unsigned int limit) {
  using namespace Gecode::Support;
  assert(limit < Support::BitSetData::data(b.sz));
  BitSetData* b_data = b.data;
  for (int i = 0; i <= limit; i++) {
    b_data[i].init(false);
  }
}

forceinline void
BitSet::flip_by_map(BitSet& b, const int* map, unsigned int map_last) {
  using namespace Gecode::Support;
  BitSetData* b_data = b.data;
  for (int i = 0; i <= map_last; i++) {
    int offset = map[i];
    assert(offset < Support::BitSetData::data(b.sz));
    BitSetData new_word = BitSetData::reverse(b_data[offset]);
    b_data[offset] = new_word;
  }
}

forceinline unsigned int
BitSet::size() const {
  return sz;
}

/** Debugging purpose **/

forceinline void
BitSet::print() const {
  for (unsigned int i = 0; i < sz; i++) {
    if (i % get_bpb() == 0) {
      printf("\n");
    }
    std::cout << get(i) << " ";
  }
  std::cout << std::endl;
}
