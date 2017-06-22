/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *
 *  Last modified:
 *     $Date: ? $ by $Author: ? $
 *     $Revision: ? $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/*
 * Threshold value for using hash table
 * Defined as domain-width / domain-size for each variable
 * (0->always hash, infinity->never hash)
 */
#define HASHH_THRESHOLD 3

namespace Gecode { namespace Int { namespace Extensional {

      /*
       * Bit-set
       *
       */
      template<class View>
      forceinline
      CompactTable<View>::BitSet::
      BitSet(void) {
        sz = 0;
      }

      template<class View>
      template<class A>
      forceinline
      CompactTable<View>::BitSet::
      BitSet(A& a,unsigned int sz,bool setbits)
        : BitSetBase(a,sz,setbits) {
        // Clear bit sz (set in RawBitSetBase)
        Gecode::Support::RawBitSetBase::clear(sz);
      }

      template<class View>
      template<class A>
      forceinline
      CompactTable<View>::BitSet::
      BitSet(A& a, const BitSet& bs)
        : BitSetBase(a,bs) {
        // Clear bit sz
        Gecode::Support::RawBitSetBase::clear(sz);
      }

      template<class View>
      template<class A>
      forceinline
      CompactTable<View>::BitSet::
      BitSet(A& a, unsigned int sz, const BitSet& bs)
        : BitSetBase(a,sz) {
        assert(sz <= bs.sz);
        for (unsigned int i = Gecode::Support::BitSetData::data(sz+1); i--; )
          data[i] = bs.data[i];
        // Clear bit sz
        Gecode::Support::RawBitSetBase::clear(sz);
      }

      template<class View>
      template<class A>
      forceinline void
      CompactTable<View>::BitSet::
      allocate(A& a, unsigned int sz0) {
        RawBitSetBase::allocate(a,sz0);
        sz = sz0;
      }

      template<class View>
      forceinline bool
      CompactTable<View>::BitSet::
      empty() const {
        return sz == 0;
      }

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      copy(unsigned int sz0, const BitSet& bs) {
        Gecode::Support::RawBitSetBase::copy(sz0,bs);
      }

      template<class View>
      forceinline bool
      CompactTable<View>::BitSet::
      same(Gecode::Support::BitSetData d, unsigned int i) const {
        assert(i < Support::BitSetData::data(sz));
        return data[i].Gecode::Support::BitSetData::same(d);
      }

      template<class View>
      template<class A>
      forceinline void
      CompactTable<View>::BitSet::
      dispose(A& a) {
        RawBitSetBase::dispose(a,sz);
      }

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      o(const BitSet& a, unsigned int i) {
        assert(i < Support::BitSetData::data(sz));
        assert(i < Support::BitSetData::data(a.sz));
        data[i].o(a.data[i]);
      }

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      a(const BitSet& a, unsigned int i) {
        assert(i < Support::BitSetData::data(sz));
        assert(i < Support::BitSetData::data(a.sz));
        data[i].a(a.data[i]);
      }

      template<class View>
      forceinline Gecode::Support::BitSetData
      CompactTable<View>::BitSet::
      o(const BitSet& a, const BitSet& b, unsigned int i) {
        assert(i < Support::BitSetData::data(a.sz));
        assert(i < Support::BitSetData::data(b.sz));
        return Gecode::Support::BitSetData::o(a.data[i], b.data[i]);
      }

      template<class View>
      forceinline Gecode::Support::BitSetData
      CompactTable<View>::BitSet::
      a(const BitSet& a, const BitSet& b, unsigned int i) {
        assert(i < Support::BitSetData::data(a.sz));
        assert(i < Support::BitSetData::data(b.sz));
        return Gecode::Support::BitSetData::a(a.data[i], b.data[i]);;
      }

      template<class View>
      forceinline Gecode::Support::BitSetData
      CompactTable<View>::BitSet::
      a(const BitSet& a, unsigned int i,
        const BitSet& b, unsigned int j) {
        assert(i < Support::BitSetData::data(a.sz));
        assert(j < Support::BitSetData::data(b.sz));
        return Gecode::Support::BitSetData::a(a.data[i], b.data[j]);;
      }


      template<class View>
      forceinline Gecode::Support::BitSetData
      CompactTable<View>::BitSet::
      getword(unsigned int i) const {
        assert(i < Support::BitSetData::data(sz));
        return data[i];
      }

      template<class View>
      forceinline int
      CompactTable<View>::BitSet::
      get_bpb() {
        return bpb;
      }

      template<class View>
      template<class A>
      forceinline void
      CompactTable<View>::BitSet::
      init(A& a, unsigned int s, bool setbits) {
        assert(sz == 0);
        RawBitSetBase::init(a,s,setbits);
        sz=s;
        // Clear sentinel bit
        Gecode::Support::RawBitSetBase::clear(sz);
      }

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      or_by_map(BitSet& a, const BitSet& b,
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

      template<class View>
      forceinline int
      CompactTable<View>::BitSet::
      intersect_index_by_map(const BitSet& a, const BitSet& b,
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

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      intersect_by_map(BitSet& a, const BitSet& b,
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

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      intersect_by_map_sparse(BitSet& a, const BitSet& b,
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

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      intersect_by_map_sparse_two(BitSet& a, const BitSet& b1,
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


      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      nand_by_map_one(BitSet& a, const BitSet& b,
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

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      nand_by_map_two(BitSet& a,
                      const BitSet& b1, const BitSet& b2,
                      int* map, int* map_last) {
        using namespace Gecode::Support;
        BitSetData* a_data = a.data;
        BitSetData* b1_data = b1.data;
        BitSetData* b2_data = b2.data;
        int local_map_last = *map_last;
        assert(local_map_last < Support::BitSetData::data(a.sz));
        for (int i = local_map_last; i >= 0; i--) {
          int offset = map[i];
          assert(offset < Support::BitSetData::data(b1.sz));
          assert(offset < Support::BitSetData::data(b2.sz));
          BitSetData flipped = BitSetData::reverse(BitSetData::o(b1_data[offset],
                                                                 b2_data[offset]));
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


      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      clear_to_limit(BitSet& b, unsigned int limit) {
        using namespace Gecode::Support;
        assert(limit < Support::BitSetData::data(b.sz));
        BitSetData* b_data = b.data;
        for (int i = 0; i <= limit; i++) {
          b_data[i].init(false);
        }
      }

      template<class View>
      forceinline void
      CompactTable<View>::BitSet::
      flip_by_map(BitSet& b, const int* map, unsigned int map_last) {
        using namespace Gecode::Support;
        BitSetData* b_data = b.data;
        for (int i = 0; i <= map_last; i++) {
          int offset = map[i];
          assert(offset < Support::BitSetData::data(b.sz));
          BitSetData new_word = BitSetData::reverse(b_data[offset]);
          b_data[offset] = new_word;
        }
      }

      template<class View>
      forceinline unsigned int
      CompactTable<View>::BitSet::
      size() const {
        return sz;
      }


      /*
       * Advisor
       *
       */
      // TODO: remove offset parameter!
      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::
      CTAdvisor(Space& home, Propagator& p, Council<CTAdvisor>& c,
                View x0, int i, BitSet* s0, int nsupports,
                int offset, IndexType type)
        : ViewAdvisor<View>(home,p,c,x0), index(i),
        supports(s0,nsupports,offset,type,x0) {}

      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::
      CTAdvisor(Space& home, CTAdvisor& a)
        : ViewAdvisor<View>(home,a), supports(a.supports), index(a.index) {}

      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::
      dispose(Space& home, Council<CTAdvisor>& c) {
        (void) supports.~Supports();
        (void) ViewAdvisor<View>::dispose(home,c);
      }

      /*
       * Shared handle for supports
       *
       */
      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::Supports::
      Supports(void) {}

      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::Supports::
      Supports(BitSet* s, int nsupports, int offset, IndexType t, View x)
        : SharedHandle(new SupportsI()) {
        static_cast<SupportsI*>(object())->
          init(s,nsupports,offset,t,x);
      }

      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::Supports::
      Supports(const Supports& s)
        : SharedHandle(s) {}

      template<class View>
      forceinline int
      CompactTable<View>::CTAdvisor::Supports::
      row(int val) {
        return static_cast<SupportsI*>(object())->info->row(val);
      }

      template<class View>
      forceinline const typename CompactTable<View>::BitSet&
      CompactTable<View>::CTAdvisor::Supports::
      operator [](unsigned int i) {
        const SupportsI* si = static_cast<SupportsI*>(object());
        return si->info->get_supports(i);
      }

      /*
       * Shared handle object
       *
       */
      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::
      init(const BitSet* s, int nsupports, int offset, IndexType t, View x) {
        type = t;
        switch (type) {
        case ARRAYY:  {
          info = heap.alloc<InfoArray>(1);
          static_cast<InfoArray*>(info)->
            InfoArray::init(s,nsupports,offset,x);
          break;
        }
        case HASHH: {
          info = heap.alloc<InfoHash>(1);
          static_cast<InfoHash*>(info)->
            InfoHash::init(s,nsupports,offset,x);
          break;
        }
        default:
          GECODE_NEVER;
          break;
        }
      }

      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::Supports::SupportsI::
      ~SupportsI(void) {
        switch (type) {
        case ARRAYY: {
          static_cast<InfoArray*>(info)->~InfoArray();
          break;
        }
        case HASHH: {
          static_cast<InfoHash*>(info)->~InfoHash();
          break;
        }
        default:
          GECODE_NEVER;
          break;
        }
        heap.rfree(info);
      }

      /*
       * Indexing supports
       *
       */
      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoBase::
      allocate(int n) {
        nvals = n;
        supports = heap.alloc<BitSet>(nvals);
      }

      template<class View>
      forceinline const typename CompactTable<View>::BitSet&
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoBase::
      get_supports(int i) {
        assert(i >= 0);
        return supports[i];
      }

      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoBase::
      ~InfoBase(void) {
        for (int i = nvals; i--; ) {
          (void) supports[i].dispose(heap);
        }
        heap.rfree(supports);
      }

      template<class View>
      forceinline int
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoBase::
      row(int n) {
        GECODE_NEVER;
        return -1;
      }

      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoBase::
      init(const BitSet* s,int nsupports, int offset,View x) {
        GECODE_NEVER;
      }

      /*
       * Indexing with sparse array
       *
       */
      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoArray::
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

      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoArray::
      allocate(int n) {
        InfoBase::allocate(n);
      }

      template<class View>
      forceinline int
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoArray::
      row(int val) {
        return val >= min && val <= max ? val - min : -1;
      }

      /*
       * Indexing with hash table
       *
       */
      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoHash::
      allocate(int n) {
        InfoBase::allocate(n);
        index_table.allocate(n);
      }

      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoHash::
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
      }

      template<class View>
      forceinline int
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoHash::
      row(int val) {
        return index_table.get(val);
      }

      /*
       * Hash table
       *
       */
      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoHash::HashTable::
      allocate(int pop) {
        size=2;
        while (size <= 2*pop)
          size *= 2;

        table = heap.alloc<HashNode>(size);
        mask = size-1;
        factor = 0.618 * size;
        for (int i = 0; i < size; i++)
          (&table[i])->value = -1; 	/* mark as free */
      }

      template<class View>
      forceinline void
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoHash::HashTable::
      insert(int key, int value) {
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

      template<class View>
      forceinline int
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoHash::HashTable::
      get(int key) const {
        long t0 = key*factor;
        int inc=0;

        while (1) {
          HashNode* hnode = &table[t0 & mask];
          if (hnode->key == key || hnode->value == -1)
            return hnode->value;
          inc++;
          t0 += inc;
        }
      }

      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::Supports::SupportsI::InfoHash::HashTable::
      ~HashTable(void) {
        heap.rfree(table);
      }

      /*
       * The propagator proper
       *
       */
      template<class View>
      forceinline
      CompactTable<View>::
      CompactTable(Space& home, CompactTable& p)
        : Propagator(home,p),
          status(NOT_PROPAGATING),
          touched_var(-1),
          arity(p.arity),
          unassigned(p.unassigned),
          max_dom_size(p.max_dom_size),
          limit(p.limit)
      {
        // Update advisors
        c.update(home,p.c);
        index = home.alloc<int>(limit + 1);
        for (int i = limit+1; i--; )
          index[i] = p.index[i];
        unsigned int nbits = (limit + 1) * CompactTable<View>::BitSet::
          get_bpb();
        words.allocate(home, nbits);
        words.copy(nbits, p.words);
        assert(limit <= Support::BitSetData::data(words.size()) - 1);
      }

      template<class View>
      forceinline
      CompactTable<View>::
      CompactTable(Home home, ViewArray<View>& x, TupleSet t)
        : Propagator(home), c(home),
          status(NOT_PROPAGATING),
          touched_var(-1),
          arity(x.size()),
          unassigned(x.size())
      {
        // Initialise supports and post advisors
        int nsupports = init_supports(home, t, x);
        if (nsupports <= 0) {
          home.fail();
          return;
        }
        init_sparse_bit_set(home, nsupports);
        // Because we use heap allocated data in advisors
        home.notice(*this,AP_DISPOSE);
        // Schedule in case we can subsume
        if (unassigned <= 1)
          View::schedule(home,*this,Int::ME_INT_VAL);
      }

      template<class View>
      PropCost
      CompactTable<View>::
      cost(const Space& home, const ModEventDelta& med) const {
        // TODO
        if (View::me(med) == ME_INT_VAL)
          return PropCost::quadratic(PropCost::HI,arity);
        else
          return PropCost::cubic(PropCost::HI,arity);
      }

      template<class View>
      void
      CompactTable<View>::
      reschedule(Space& home) {
        View::schedule(home,*this,ME_INT_DOM);
      }

      template<class View>
      ExecStatus
      CompactTable<View>::
      propagate(Space& home, const ModEventDelta& med) {
        status = PROPAGATING;
        if (is_empty())
          return ES_FAILED;

        assert(not_failed());
        assert(limit >= 0);

        ExecStatus msg = filter_domains(home);

        touched_var = -1;
        status = NOT_PROPAGATING;

        assert(limit >= 0);
        assert(not_failed());

        return msg;
      }

      template<class View>
      Actor*
      CompactTable<View>::
      copy(Space& home) {
        return new (home) CompactTable(home,*this);
      }

      template<class View>
      forceinline ExecStatus
      CompactTable<View>::
      post(Home home, ViewArray<View>& x, const TupleSet& t) {
        // All variables in the correct domain
        for (int i = x.size(); i--; ) {
          GECODE_ME_CHECK(x[i].gq(home, t.min()));
          GECODE_ME_CHECK(x[i].lq(home, t.max()));
        }
        (void) new (home) CompactTable(home,x,t);
        if (home.failed())
          return ES_FAILED;
        return ES_OK;
      }

      template<class View>
      forceinline size_t
      CompactTable<View>::
      dispose(Space& home) {
        home.ignore(*this,AP_DISPOSE);
        c.dispose(home);
        (void) Propagator::dispose(home);
        return sizeof(*this);
      }

      template<class View>
      forceinline unsigned int
      CompactTable<View>::
      init_supports(Space& home, TupleSet t, ViewArray<View>& x) {
        // Find maximum domain size and total domain width
        max_dom_size = 1;
        unsigned int domsum = 0;
        for (int i = x.size(); i--; ) {
          domsum += x[i].width();
          if (x[i].size() > max_dom_size)
            max_dom_size = x[i].size();
        }

        Region r;
        // Allocate temporary supports and residues
        BitSet* supports = r.alloc<BitSet>(domsum);

        // Save initial minimum value and widths for indexing supports and residues
        int* min_vals = r.alloc<int>(x.size());
        int* offset = r.alloc<int>(x.size());
        for (int i = 0; i<x.size(); i++) {
          min_vals[i] = x[i].min();
          offset[i] = i != 0 ? offset[i-1] + x[i-1].width() : 0;
        }

        int support_cnt = 0;
        int bpb = CompactTable<View>::BitSet::
          get_bpb(); // Bits per base (word) in bitsets

        // Look for supports and set correct bits in supports
        for (int i = 0; i < t.tuples(); i++) {
          bool supported = true;
          for (int j = t.arity(); j--; ) {
            if (!x[j].in(t[i][j])) {
              supported = false;
              break;
            }
          }
          if (supported) {
            // Set tuple as valid and save word index in residue
            for (int j = t.arity(); j--; ) {
              int val = t[i][j];
              unsigned int row = offset[j] + val - min_vals[j];

              if (supports[row].empty()) // Initialise in case not done
                supports[row].init(r,t.tuples(),false);

              supports[row].set(support_cnt);
            }
            support_cnt++;
          }
        }

        int* nq = r.alloc<int>(max_dom_size);
        int nremoves;

        // Remove values corresponding to empty rows
        for (int i = x.size(); i--; ) {
          nremoves = 0;

          Int::ViewValues<View> it(x[i]);
          while (it()) {
            unsigned int row = offset[i] + it.val() - min_vals[i];
            if (supports[row].size() == 0) {
              nq[nremoves++] = it.val();
            }
            ++it;
          }
          Iter::Values::Array r(nq,nremoves);
          GECODE_ME_CHECK(x[i].minus_v(home,r,false));
        }

        // Post advisors
        for (int i = x.size(); i--; ) {
          if (!x[i].assigned()) {
            // Decide whether to use an array or a hash table
            double sparseness = x[i].width() / x[i].size();
            IndexType type = sparseness < HASHH_THRESHOLD ? ARRAYY : HASHH;

            // To shift the offset
            int diff = x[i].min() - min_vals[i];

            (void) new (home) CTAdvisor(home,*this,c,x[i],i,
                                        supports,
                                        support_cnt,
                                        offset[i] + diff,
                                        type);
          } else
            unassigned--;
        }
        return support_cnt;
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      init_sparse_bit_set(Space& home, unsigned int s) {
        words.init(home,s);
        int nwords = s != 0 ? (s - 1) / words.get_bpb() + 1 : 0;
        limit = nwords - 1;
        index = home.alloc<int>(nwords);
        for (int i = limit+1; i--; )
          index[i] = i;
        // Set the first s nr of bits in words
        int start_bit = 0;
        int complete_words = s / CompactTable<View>::BitSet::
          get_bpb();
        if (complete_words > 0) {
          start_bit = complete_words * CompactTable<View>::BitSet::
            get_bpb() + 1;
          words.Gecode::Support::RawBitSetBase::clearall(start_bit - 1,true);
        }
        for (unsigned int i = start_bit; i < s; i++) {
          words.set(i);
        }

        assert(not_failed());
        assert(limit >= 0);
        assert(limit <= Support::BitSetData::data(words.size()) - 1);
      }

      template<class View>
      forceinline ExecStatus
      CompactTable<View>::
      filter_domains(Space& home) {
        if (unassigned == 0)
          return home.ES_SUBSUMED(*this);

        assert(limit >= 0);
        // Count the number of scanned unassigned variables
        unsigned int count_unassigned = unassigned;
        // Array to collect values to remove
        Region r;
        int* nq = r.alloc<int>(max_dom_size);
        int* nq_start = nq;

        // Scan all values of all unassigned variables to see if they
        // are still supported.
        for (Advisors<CTAdvisor> a0(c);
             a0() && count_unassigned; // End if only assigned variables left
             ++a0) {
          CTAdvisor& a = a0.advisor();
          View v = a.view();
          int i = a.index;

          // No point filtering variable if it was the only modified variable
          if (touched_var == i) {
            continue;
          }

          switch (v.size()) {
          case 1: {  // Variable assigned, nothing to be done.
            break;
          }
          case 2: { // Consider min and max values
            const int min_val = v.min();
            const int max_val = v.max();

            // Fix to max_val if min_val not supported
            const int row_min = a.supports.row(min_val);
            if (!supported(a,row_min)) {
              GECODE_ME_CHECK(v.eq(home,max_val));
              --unassigned;
              break;
            }

            // Fix to min_val if max_val not supported
            const int row_max = a.supports.row(max_val);
            if (!supported(a,row_max)) {
              GECODE_ME_CHECK(v.eq(home,min_val));
              --unassigned;
              break;
            }

            // Otherwise v is still unassigned
            count_unassigned--;
            break;
          } default:
            Int::ViewRanges<View> rngs(v);
            int cur, max, row;
            int last_support;
            nq = nq_start;
            while (rngs()) {
              cur = rngs.min();
              max = rngs.max();
              row = a.supports.row(cur);
              while (cur <= max) {
                assert(v.in(cur));
                if (!supported(a,row))
                  *(nq++) = cur;
                else
                  last_support = cur;
                ++cur;
                ++row;
              }
              ++rngs;
            }
            unsigned int nremoves = static_cast<unsigned int>(nq - nq_start);

            // Remove collected values
            if (nremoves > 0) {
              assert(nremoves < v.size());
              if (nremoves == v.size() - 1) {
                GECODE_ME_CHECK(v.eq(home,last_support));

                --unassigned;
                break;
              } else {
                Iter::Values::Array r(nq_start,nremoves);
                ModEvent me = v.minus_v(home,r,false);
                if (me_failed(me))
                  return ES_FAILED;
                if (me == ME_INT_VAL) {
                  --unassigned;
                  break;
                }
              }
            }
            --count_unassigned;
          }
        }
        assert(not_failed());
        // Subsume if there is at most one non-assigned variable
        return unassigned <= 1 ? home.ES_SUBSUMED(*this) : ES_FIX;
      }

      template<class View>
      forceinline bool
      CompactTable<View>::
      is_empty(void) {
        return limit == -1;
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      clear_mask(BitSet& mask) {
        assert(limit >= 0);
        assert(not_failed());
        CompactTable<View>::BitSet::
          clear_to_limit(mask, static_cast<unsigned int>(limit));
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      add_to_mask(const BitSet& b, BitSet& mask) const {
        assert(limit >= 0);
        assert(not_failed());
        CompactTable<View>::BitSet::
          or_by_map(mask, b, index, static_cast<unsigned int>(limit));
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      intersect_with_mask_compressed(const BitSet& mask) {
        assert(limit >= 0);
        assert(not_failed());
        CompactTable<View>::BitSet::
          intersect_by_map(words,mask,index,&limit);
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      intersect_with_mask_sparse_one(const BitSet& mask) {
        assert(limit >= 0);
        assert(not_failed());
        CompactTable<View>::BitSet::
          intersect_by_map_sparse(words,mask,index,&limit);
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      intersect_with_mask_sparse_two(const BitSet& a, const BitSet& b) {
        assert(limit >= 0);
        assert(not_failed());
        CompactTable<View>::BitSet::
          intersect_by_map_sparse_two(words,a,b,index,&limit);
      }

      template<class View>
      forceinline int
      CompactTable<View>::
      intersect_index(const BitSet& b, int max_index) {
        assert(limit >= 0);
        assert(max_index >= 0);
        assert(max_index <= limit);
        assert(not_failed());
        return CompactTable<View>::BitSet::
          intersect_index_by_map(words,b,index,
                                 static_cast<unsigned int>(max_index));
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      nand_with_mask_one(const BitSet& b) {
        assert(limit >= 0);
        assert(not_failed());
        CompactTable<View>::BitSet::
          nand_by_map_one(words,b,index,&limit);
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      nand_with_mask_two(const BitSet& a, const BitSet& b) {
        assert(limit >= 0);
        assert(not_failed());
        CompactTable<View>::BitSet::
          nand_by_map_two(words,a,b,index,&limit);
      }

      template<class View>
      ExecStatus
      CompactTable<View>::
      advise(Space& home, Advisor& a0, const Delta& d) {
        CTAdvisor& a = static_cast<CTAdvisor&>(a0);
        View x = a.view();

        // Do not fail a disabled propagator
        if (is_empty())
          return disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;

        assert(limit >= 0);
        assert(not_failed());

        // Do not schedule if propagator is performing propagation,
        // and dispose if assigned
        if (status == PROPAGATING) {
          if (x.assigned())
            return home.ES_FIX_DISPOSE(c,a);
          return ES_FIX;
        }

        ModEvent me = View::modevent(d);
        if (me == ME_INT_VAL) { // Variable is assigned -- intersect with its value
          int row = a.supports.row(x.val());
          intersect_with_mask_sparse_one(a.supports[row]);
        }
        else if (x.any(d)){ // No delta information -- do incremental update
          reset_based_update(a,home);
        } else { // Delta information available -- let's compare the size of
          // the domain with the size of delta to decide whether or not
          // to do reset-based or incremental update
          int min_rm = x.min(d);
          int max_rm = x.max(d);
          int min_row = a.supports.row(min_rm);
          int max_row = a.supports.row(max_rm);
          // Push min_row and max_row to closest corresponding tabulated values.
          // This happens if min_rm or max_rm were not in the domain of x
          // when the advisor was posted. Those values need not be considered since
          // we were at fixpoint when the advisor was posted.
          while (min_row == -1) // -1 means value is not tabulated
            min_row = a.supports.row(++min_rm);
          while (max_row == -1)
            max_row = a.supports.row(--max_rm);
          assert(max_row >= min_row);

          if (static_cast<unsigned int>(max_row - min_row + 1) <= x.size()) { // Delta is smaller
            for (int i = min_row; i <= max_row; i+=2) { // Process supports two and two
              if (i != max_row) {                 // At least two values left
                assert(i + 1 <= max_row);
                const BitSet& s1 = a.supports[i];
                const BitSet& s2 = a.supports[i+1];
                if (!s1.empty() && !s2.empty()) { // Both non-empty
                  nand_with_mask_two(s1,s2);
                } else if (!s1.empty()) {         // s1 non-empty, s2 empty
                  nand_with_mask_one(s1);
                } else if (!s2.empty()) {         // s2 non-empty, s1 empty
                  nand_with_mask_one(s2);
                }
              } else {                            // Last value
                assert(static_cast<unsigned int>(max_row - min_row + 1) % 2 == 1);
                const BitSet& s = a.supports[i];
                if (!s.empty())
                  nand_with_mask_one(s);
              }
            }
          } else { // Domain size smaller than delta, reset-based update
            reset_based_update(a,home);
          }
        }

        // Do not fail a disabled propagator
        if (is_empty())
          return disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;

        assert(limit >= 0);
        assert(not_failed());

        // Update touched_var
        if (touched_var == -1) // no touched variable yet!
          touched_var = a.index;
        else if (touched_var != a.index) // some other variable is touched
          touched_var = -2;

        // Schedule propagator and dispose if assigned
        if (a.view().assigned()) {
          unassigned--;
          return home.ES_NOFIX_DISPOSE(c,a);
        }
        return ES_NOFIX;

      }

      template<class View>
      forceinline void
      CompactTable<View>::
      reset_based_update(CTAdvisor a, Space& home) {
        assert(not_failed());
        assert(limit >= 0);
        assert(a.view().size() >= 2);
        switch (a.view().size()) {
        case 2: {
          // Intersect with validTuples directly
          int row_min = a.supports.row(a.view().min());
          int row_max = a.supports.row(a.view().max());
          intersect_with_mask_sparse_two(a.supports[row_min],
                                         a.supports[row_max]);
          break;
        }
        default:
          // Collect all tuples to be kept in a temporary mask
          Region r;
          BitSet mask;
          mask.allocate(r,words.size());

          Int::ViewRanges<View> rngs(a.view());

          clear_mask(mask);
          int cur, max, row;
          while (rngs()) {
            cur = rngs.min();
            max = rngs.max();
            row = a.supports.row(cur);
            while (cur <= max) {
              assert(a.view().in(cur));
              add_to_mask(a.supports[row],mask);
              ++cur;
              ++row;
            }
            ++rngs;
          }
          intersect_with_mask_compressed(mask);
          break;
        }
      }

      template<class View>
      forceinline bool
      CompactTable<View>::
      supported(CTAdvisor& a, int row) {
        int r = intersect_index(a.supports[row], limit);
        return r >= 0;
      }

      template<class View>
      bool
      CompactTable<View>::
      not_failed(void) const {
        for (int i = 0; i <= limit; i++) {
          if (words.getword(i).none()) {
            printf("Word %d is zero\n", i);
            return false;
          }
        }

        int count = 0;
        for (int i = 0; i <= limit; i++) {
          for (int j = 0; j < CompactTable<View>::BitSet::
get_bpb(); j++) {
            count += words.get(i*CompactTable<View>::BitSet::
get_bpb() + j);
            if (i*CompactTable<View>::BitSet::
get_bpb() + j == words.size() - 1) {
              break;
            }
          }
        }

        if (count <= 0) {
          printf("count = %d\n", count);
        }
        return count > 0;

      }
    }}}
