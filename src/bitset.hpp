class BitSet : public Gecode::Support::BitSetBase {
public:
  /// Perform "or" with \a a of word index \a i
  void o(BitSet a, unsigned int i);
  /// Perform "and" with \a a of word index \a i
  void a(BitSet a, unsigned int i);
  /// Return "or" of \a a and \a b of word index \a i
  static Gecode::Support::BitSetData o(BitSet a, BitSet b, unsigned int i);
  /// Return "and" of \a a and \a b of word index \a i
  static Gecode::Support::BitSetData a(BitSet a, BitSet b, unsigned int i);
  /// Clear all bits at word index \a i
  void clearword(unsigned int i, bool setbits);
  /// Set a word \a i to \a a
  void setword(Gecode::Support::BitSetData a, unsigned int i);
  /// Check if bit set has \a d on index \a i
  bool same(Gecode::Support::BitSetData d, unsigned int i);
  /// Get number of bits per base
  static int get_bpb();
};

template<class A>
class NewSparseBitSet {
private:
  /// Allocator
  A& a;
  /// Words
  BitSet words;
  /// Mask
  BitSet mask;
  /// Index
  unsigned int* index;
  /// Limit
  int limit;
public:
  /// Sparse bit set with space for \a s bits and allocator \a a
  NewSparseBitSet(A& a, unsigned int s);
  /// Copy sparse bit set \a sbs with allocator \a a
  NewSparseBitSet(A& a, const NewSparseBitSet& sbs);
  /// Destructor
  ~NewSparseBitSet(void);
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
};

/**
 * Sparse bit set
 */
template<class A>
forceinline
NewSparseBitSet<A>::NewSparseBitSet(A& a0, unsigned int s)
  : a(a0) {
  // Calculate number of required words
  int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
  words.init(a, s, false);
  mask.init(a, s, false);
  index = a.template alloc<int>(nwords);
  limit = nwords - 1;
  for (int i = 0; i <= limit; i++) {
    index[i] = i;
  }
}

template<class A>
forceinline
NewSparseBitSet<A>::NewSparseBitSet(A& a0, const NewSparseBitSet<A>& sbs)
  : a(a0), limit(sbs.limit) {
  int s = sbs.words.sz;
  int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
  words.init(a, s, false);
  mask.init(a, s, false);
  index = a.template alloc<int>(limit + 1);
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
    Gecode::Support::BitSetData w = a(words, mask, offset);
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
    if (!a(words, b, offset).none()) {
      return offset;
    }
    return -1;
  }
}

template<class A>
forceinline
NewSparseBitSet<A>::~NewSparseBitSet(void) {
  dispose(a);
}

/**
 * Bit set
 */
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

forceinline int
BitSet::get_bpb() {
  return bpb;
}

forceinline bool
BitSet::same(Gecode::Support::BitSetData d, unsigned int i) {
  return data[i].same(d);
}
