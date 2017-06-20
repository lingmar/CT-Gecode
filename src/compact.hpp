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
       * Advisor
       *
       */
      // TODO: remove offset parameter!
      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::
      CTAdvisor(Space& home, Propagator& p, Council<CTAdvisor>& c,
                View x0, int i, BitSet* s0, unsigned int* res, int nsupports,
                int offset, IndexType type)
        : ViewAdvisor<View>(home,p,c,x0), index(i), offset(0),
        supports(s0,nsupports,offset,type,x0)
      {
        // Initialise residues
        switch (type) {
        case ARRAYY: {
          // Sparse array
          int nvals = x0.max() - x0.min() + 1;
          residues = home.alloc<unsigned int>(nvals);
          for (unsigned int i = 0; i < nvals; i++)
            residues[i] = res[i + offset];
          break;
        }
        case HASHH: {
          // Pack the residues tight
          int nvals = x0.size();
          residues = home.alloc<unsigned int>(nvals);
          int count = 0;
          int diff = x0.min();

          Int::ViewValues<View> it(x0);
          while (it()) {
            residues[count] = res[it.val() + offset - diff];
            ++count;
            ++it;
          }
          break;
        }
        default:
          GECODE_NEVER;
          break;
        }
      }

      template<class View>
      forceinline
      CompactTable<View>::CTAdvisor::
      CTAdvisor(Space& home, CTAdvisor& a)
        : ViewAdvisor<View>(home,a), supports(a.supports)
      {
        View x = a.view();
        if (!x.assigned()) {
          index = a.index;
          // Copy residues
          const int min_row = supports.row(x.min()) - a.offset;
          const int max_row = supports.row(x.max()) - a.offset;
          offset = a.offset + min_row;
          residues = home.alloc<unsigned int>(x.width());
          int cnt = 0;
          for (int i = min_row; i <= max_row; i++)
            residues[cnt++] = a.residues[i];
        }
      }

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
      forceinline const BitSet&
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
      forceinline const BitSet&
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
        unsigned int nbits = (limit + 1) * BitSet::get_bpb();
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
        unsigned int* residues = r.alloc<unsigned int>(domsum);

        // Save initial minimum value and widths for indexing supports and residues
        int* min_vals = r.alloc<int>(x.size());
        int* offset = r.alloc<int>(x.size());
        for (int i = 0; i<x.size(); i++) {
          min_vals[i] = x[i].min();
          offset[i] = i != 0 ? offset[i-1] + x[i-1].width() : 0;
        }

        int support_cnt = 0;
        int bpb = BitSet::get_bpb(); // Bits per base (word) in bitsets

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
              // Save the index in words where a support is found for the value
              residues[row] = support_cnt / bpb;
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
                                        residues,
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
        int complete_words = s / BitSet::get_bpb();
        if (complete_words > 0) {
          start_bit = complete_words * BitSet::get_bpb() + 1;
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
            const unsigned int offset = a.offset;

            // Fix to max_val if min_val not supported
            const int row_min = a.supports.row(min_val);
            if (!supported(a,row_min,offset)) {
              GECODE_ME_CHECK(v.eq(home,max_val));
              --unassigned;
              break;
            }

            // Fix to min_val if max_val not supported
            const int row_max = a.supports.row(max_val);
            if (!supported(a,row_max,offset)) {
              GECODE_ME_CHECK(v.eq(home,min_val));
              --unassigned;
              break;
            }

            // Otherwise v is still unassigned
            count_unassigned--;
            break;
          } default:
            unsigned int offset = a.offset;
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
                if (!supported(a,row,offset))
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
        BitSet::clear_to_limit(mask, static_cast<unsigned int>(limit));
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      add_to_mask(const BitSet& b, BitSet& mask) const {
        assert(limit >= 0);
        assert(not_failed());
        BitSet::or_by_map(mask, b, index, static_cast<unsigned int>(limit));
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      intersect_with_mask_compressed(const BitSet& mask) {
        assert(limit >= 0);
        assert(not_failed());
        BitSet::intersect_by_map(words,mask,index,&limit);
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      intersect_with_mask_sparse_one(const BitSet& mask) {
        assert(limit >= 0);
        assert(not_failed());
        BitSet::intersect_by_map_sparse(words,mask,index,&limit);
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      intersect_with_mask_sparse_two(const BitSet& a, const BitSet& b) {
        assert(limit >= 0);
        assert(not_failed());
        BitSet::intersect_by_map_sparse_two(words,a,b,index,&limit);
      }

      template<class View>
      forceinline int
      CompactTable<View>::
      intersect_index(const BitSet& b, int max_index) {
        assert(limit >= 0);
        assert(max_index >= 0);
        assert(max_index <= limit);
        assert(not_failed());
        return BitSet::intersect_index_by_map(words,b,index,
                                              static_cast<unsigned int>(max_index));
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      nand_with_mask_one(const BitSet& b) {
        assert(limit >= 0);
        assert(not_failed());
        BitSet::nand_by_map_one(words,b,index,&limit);
      }

      template<class View>
      forceinline void
      CompactTable<View>::
      nand_with_mask_two(const BitSet& a, const BitSet& b) {
        assert(limit >= 0);
        assert(not_failed());
        BitSet::nand_by_map_two(words,a,b,index,&limit);
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
      supported(CTAdvisor& a, int row, unsigned int offset) {
        int r = static_cast<int>(a.residues[row - offset]);
        const BitSet& support_row = a.supports[row];

        if (r == 0)
          return !BitSet::a(words,r,support_row,index[r]).none();

        if (r > limit)
          r = intersect_index(support_row, limit);
        else if (!BitSet::a(words,r,support_row,index[r]).none())
          return true;
        else
          r = intersect_index(support_row, r-1);

        if (r != -1) {
          assert(r >= 0);
          a.residues[row - offset] = static_cast<unsigned int>(r);
          return true;
        }
        return false;
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
          for (int j = 0; j < BitSet::get_bpb(); j++) {
            count += words.get(i*BitSet::get_bpb() + j);
            if (i*BitSet::get_bpb() + j == words.size() - 1) {
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
