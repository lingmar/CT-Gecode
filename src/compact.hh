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

#ifndef __COMPACT_HH__
#define __COMPACT_HH__

#include <gecode/int.hh>

#include "/Users/linneaingmar/Documents/Kurser/exjobb/src/bitset.hpp"

/**
 * \namespace Gecode::Int::Extensional
 * \brief %Extensional propagators
 */
namespace Gecode { namespace Int { namespace Extensional {
      // Macros ARRAY and HASH are already defined on Linu
      typedef enum {ARRAYY,HASHH} IndexType;

      /**
       * \brief Domain consistent extensional propagator
       *
       * This propagator implements the compact-table propagation
       * algorithm based on:
       *   J. Demeulenaere et. al., Compact-Table: Efficiently
       *   filtering table constraints with reversible sparse
       *   bit-sets, CP 2016.
       *   Pages 207-223, LNCS, Springer, 2016.
       *
       * Requires \code #include <gecode/int/extensional.hh> \endcode
       * \ingroup FuncIntProp
       */
      template<class View>
      class CompactTable : public Propagator {
      protected:
        enum Status {NOT_PROPAGATING,PROPAGATING};
        /// Number of unassigned variables
        unsigned int unassigned;
        /// -2 if more than one touched var, -1 if none, else index of the only touched var
        int touched_var;
        /// Whether propagator is propagating or not
        Status status;
        /// Largest domain size
        unsigned int max_dom_size;
        /// Arity
        unsigned int arity;
        /// Current table
        BitSet words;
        /// Maps the current index to its original index in words
        int* index;
        /// Equals to the number of non-zero words minus 1
        int limit;

        /// Constructor for cloning \a p
        CompactTable(Space& home, CompactTable& p);
        /// Constructor for posting
        CompactTable(Home home, ViewArray<View>& x, TupleSet t);
        /// Initialize support and return number of valid tuples
        unsigned int init_supports(Space& home, TupleSet t, ViewArray<View>& x);
        /// Initialize sparse bit-set with \a s set bits
        void init_sparse_bit_set(Space& home, unsigned int s);
        /// Filter domains of views
        ExecStatus filter_domains(Space& home);
        /// Check whether current table is empty
        bool is_empty(void);
        /// Clear the first limit words in \a mask
        void clear_mask(BitSet& mask);
        /// Initialise \a mask as the "or" of \a a and \a b
        void init_mask(const BitSet& a, const BitSet& b, BitSet& mask);
        /// Add \b to \a mask
        void add_to_mask(const BitSet& b, BitSet& mask) const;
        /// Intersect words with compressed \a mask
        void intersect_with_mask_compressed(const BitSet& mask);
        /// Intersect words with sparse \a mask
        void intersect_with_mask_sparse_one(const BitSet& mask);
        /// Intersect words with the "or" of \a and \a b
        void intersect_with_mask_sparse_two(const BitSet& a, const BitSet& b);
        /// Get the index of a non-zero intersect with \a b, or -1 if none exists
        int intersect_index(const BitSet& b, int max_index);
        /// Perform "nand" with words and \a b
        void nand_with_mask_one(const BitSet& b);
        /// Perform "nand" with words and the "or" of \a a and \a b
        void nand_with_mask_two(const BitSet& a, const BitSet& b);
      public:
        /// Cost function
        virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
        /// Schedule function
        virtual void reschedule(Space& home);
        /// Perform propagation
        virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
        /// Copy propagator during cloning
        virtual Actor* copy(Space& home);
        /// Post propagator for views \a x and table \a t
        static ExecStatus post(Home home, ViewArray<View>& x, const TupleSet& t);
        /// Delete propagator and return its size
        size_t dispose(Space& home);
      private:
        /// Advisor for updating current table
        class CTAdvisor : public Gecode::ViewAdvisor<View> {
        public:
          /// Offset from initial minimum value for view
          int offset;
          /// Class for storing support information
          class Supports : public SharedHandle {
          protected:
            /// Object implementation for Supports
            class SupportsI : public SharedHandle::Object {
            public:
              /// Support information and indexing
              class InfoBase {
              protected:
                /// Support bits
                BitSet* supports;
                /// Number of values
                int nvals;
              public:
                /// Allocate for \a n values
                virtual void allocate(int n);
                /// Initialise from parameters
                virtual void init(const BitSet* supports, int nsupports, int offset, View x);
                /// Get support entry at index \a i
                virtual const BitSet& get_supports(int i);
                /// Get the row for value \a val
                virtual int row(int val);
                /// Desctructor
                virtual ~InfoBase(void);
              };
              /// Indexing with array
              class InfoArray : public InfoBase {
              private:
                using InfoBase::nvals;
                using InfoBase::supports;
                /// Initial minimum value for view
                int min;
                /// Initial maximum value for view
                int max;
              public:
                /// Allocate for \a n values
                virtual void allocate(int n);
                /// Initialise from parameters
                virtual void init(const BitSet* supports, int nsupports, int offset, View x);
                /// Get the row for value \a val
                virtual int row(int val);
              };
              /// Indexing with hash table
              class InfoHash : public InfoBase {
              private:
                using InfoBase::nvals;
                using InfoBase::supports;
              public:
                /// Allocate for \a n values
                virtual void allocate(int n);
                /// Initialise from parameters
                virtual void init(const BitSet* supports, int nsupports, int offset, View x);
                /// Get the row for value \a val
                virtual int row(int val);
                /// Hash table
              private:
                class HashTable {
                private:
                  /// Hash node
                  typedef struct HashNode {
                    /// Hash key
                    int key;
                    /// Hash value
                    int value;
                  } HashNode;
                  /// Table of hash nodes
                  HashNode* table;
                  /// Mask
                  long mask;
                  /// Factor
                  long factor;
                  /// Size
                  int size;
                public:
                  /// Allocate for \a n elements
                  void allocate(int n);
                  /// Insert entry with \a key with \a value
                  void insert(int key, int value);
                  /// Get value for \a key
                  int get(int key) const;
                  /// Destructor
                  ~HashTable(void);
                };
                /// Hash table for indexing
                HashTable index_table;
              };
              /// Type of indexing
              IndexType type;
              /// Info object
              InfoBase* info;
              /// Initialise from parameters
              void init(const BitSet* s, int n, int off, IndexType t, View x);
              /// Destructor
              virtual ~SupportsI(void);
            };
          public:
            /// \name Constructors
            //@{
            /// Default constructor
            Supports(void);
            /// Initialize with supports \a s, \a n values, type \a t, view \a x
            Supports(BitSet* s, int n, int off, IndexType t, View x);
            /// Copy \a s
            Supports(const Supports& s);
            //@}

            /// \name Indexing
            //@{
            /// Get index for value \a val
            int row(int val);
            /// Get support entry for index \a i
            const BitSet& operator [](unsigned int i);
            //@}

          };
          /// Position of view
          int index;
          /// Support information for view
          Supports supports;
          /// Word indices for the last found support for each value
          unsigned int* residues;

          /// \name Constructors
          //@{
          /// Initialise from parameters
          CTAdvisor(Space& home, Propagator& p, Council<CTAdvisor>& c,
                    View x0, int i, BitSet* s0, unsigned int* res,
                    int nsupports, int offset, IndexType type);
          /// Clone advisor \a a
          CTAdvisor(Space& home, CTAdvisor& a);
          //@}

          /// Dispose advisor
          void dispose(Space& home, Council<CTAdvisor>& c);
        };
        /// The advisor council
        Council<CTAdvisor> c;
      public:
        /// Give advice to propagator
        virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
      private:
        /// Perform reset-based update
        void reset_based_update(CTAdvisor a, Space& home);
        /// Check for support for variable with support entry in \a row
        bool supported(CTAdvisor& a, int row, unsigned int offset);
        /// Check that the propagator is not in a failed state
        bool not_failed(void) const;
      };
    }}}

//#include <gecode/int/extensional/compact.hpp>
#include "/Users/linneaingmar/Documents/Kurser/exjobb/src/compact.hpp"
#include "/Users/linneaingmar/Documents/Kurser/exjobb/src/compact.cpp"

#endif
