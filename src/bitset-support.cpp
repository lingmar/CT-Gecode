#include <gecode/kernel.hh>
#include "bitset.hpp"

//#define MEMOPT 1

using namespace Gecode;

class SharedSupports : public SharedHandle {
protected:
  class Supports : public SharedHandle::Object {
  public:
    /// Support bits
    BitSet* supports;
    /// Starting index
    unsigned int* start_idx;
    /// Starting value
    unsigned int* start_val;
    /// Number of rows
    unsigned int domsize;
    /// Number of bits in each row
    unsigned int nsupports;
    /// Number of variables
    unsigned int vars;
    /// Default constructor (empty set)
    Supports(void);
    /// Copy \a s
    Supports(const Supports& s);
    /// Allocate supports for \a d rows, \a variables and \a n bits
    void allocate_supports(unsigned int d,unsigned int v,unsigned int n);
    /// Row number for variable \a var and value \a val
    unsigned int row(unsigned int var, int val) const;
    /// Copy function
    virtual Object* copy(void) const;
    /// Destructor
    virtual ~Supports(void);
  };
public:
  /// Default constructor (yields empty set)
  SharedSupports(void);
  /// Copy \a s
  SharedSupports(const SharedSupports& s);
  /// Assignment operator
  SharedSupports& operator =(const SharedSupports& si);
  /// Get bit set for variable i, value j
  BitSet& get(unsigned int i, int j) const;
  /// Allocate supports for \a d rows, \a variables and \a n bits
  void init_supports(unsigned int d, unsigned int v, unsigned int n);
  /// Set start index for variable \a i to \a idx
  void set_start_idx(unsigned int i, unsigned int idx);
  /// Set start value for variable \a i to \a val
  void set_start_val(unsigned int i, unsigned int val);
  /// Get start index for variable \a i
  unsigned int get_start_idx(unsigned int i) const;
  /// Get start index for variable \a i
  unsigned int get_start_val(unsigned int i) const;
  
  /// Print bit set for variable \a i and value \a
  void print(unsigned int i,unsigned int j) const;
  /// Row number for variable \a var and value \a val
  unsigned int row(unsigned int var, int val) const;
  void update(Space& home, bool share, SharedHandle& sh) {
    SharedHandle::update(home,share,sh);
  }
  /// Destructor
  ~SharedSupports(void);
};

forceinline
SharedSupports::Supports::Supports(void)
  : supports(NULL), start_idx(NULL), domsize(0),
    nsupports(0), vars(0) {
}

forceinline
SharedSupports::Supports::Supports(const Supports& s)
  : domsize(s.domsize), vars(s.vars), nsupports(s.nsupports) {
  supports = heap.alloc<BitSet>(domsize);
  start_idx = heap.alloc<unsigned int>(vars);
  start_val = heap.alloc<unsigned int>(vars);
  
#ifdef MEMOPT
  // Allocate a chunk of memory to store the bit sets
  unsigned int bases_per_bs = Support::BitSetData::data(nsupports);
  Support::BitSetData* mem =
    heap.alloc<Support::BitSetData>((nsupports+1) * bases_per_bs);
  for (unsigned int i = 0; i < domsize; i++) {
    supports[i].init(&mem[i * bases_per_bs],nsupports,false);
    supports[i].copy(nsupports,s.supports[i]);
  }
#else
  for (unsigned int i = 0; i < domsize; i++) {
    supports[i].Support::BitSetBase::init(heap,nsupports,false);
    supports[i].copy(nsupports,s.supports[i]);
  }
#endif // MEMOPT
  
  for (unsigned int i = 0; i < vars; i++) {
    start_idx[i] = s.start_idx[i];
    start_val[i] = s.start_val[i];
  }
}

forceinline void
SharedSupports::Supports::allocate_supports(unsigned int d,unsigned int v,unsigned int n) {
  domsize = d;
  vars = v;
  nsupports = n;
  supports = heap.alloc<BitSet>(domsize);
  start_idx = heap.alloc<unsigned int>(vars);
  start_val = heap.alloc<unsigned int>(vars);
#ifdef MEMOPT
  // Allocate a chunk of memory to store the bit sets
  unsigned int bases_per_bs = Support::BitSetData::data(nsupports);
  Support::BitSetData* mem =
    heap.alloc<Support::BitSetData>(nsupports * bases_per_bs);
  for (unsigned int i = 0; i < d; i++) {
    //std::cout << "allocate " << i << "/" << d << std::endl;
    supports[i].init(&mem[i * bases_per_bs],nsupports,false);
  }
#else
  for (unsigned int i = 0; i < d; i++) {
    supports[i].Support::BitSetBase::init(heap,nsupports,false);
  }
#endif // MEMOPT
}

forceinline unsigned int
SharedSupports::Supports::row(unsigned int var, int val) const {
  return start_idx[var] + val - start_val[var];
}

forceinline SharedSupports::Object*
SharedSupports::Supports::copy(void) const {
  return new Supports(*this);
}

forceinline
SharedSupports::Supports::~Supports(void) {
  //  heap.rfree(supports);
  //heap.rfree(start_idx);
  //heap.rfree(start_val);
  // TODO: bitsets
}

forceinline
SharedSupports::SharedSupports(void)
  : SharedHandle(new Supports()) {}

forceinline
SharedSupports::SharedSupports(const SharedSupports& si)
  : SharedHandle(si) {}

forceinline SharedSupports&
SharedSupports::operator =(const SharedSupports& si) {
  return static_cast<SharedSupports&>(SharedHandle::operator =(si));
}

forceinline BitSet&
SharedSupports::get(unsigned int i, int j) const {
  const SharedSupports::Supports* s =
    static_cast<const SharedSupports::Supports*>(object());
  return s->supports[s->row(i,j)];
}

forceinline void
SharedSupports::init_supports(unsigned int d, unsigned int v, unsigned int n) {
  static_cast<SharedSupports::Supports*>(object())->allocate_supports(d,v,n);
}

forceinline void
SharedSupports::set_start_idx(unsigned int i, unsigned int idx) {
  static_cast<SharedSupports::Supports*>(object())->start_idx[i] = idx;
}

forceinline void
SharedSupports::set_start_val(unsigned int i, unsigned int val) {
  static_cast<SharedSupports::Supports*>(object())->start_val[i] = val;
}

forceinline unsigned int
SharedSupports::get_start_idx(unsigned int i) const {
  return static_cast<SharedSupports::Supports*>(object())->start_idx[i];
}

forceinline unsigned int
SharedSupports::get_start_val(unsigned int i) const {
  return static_cast<SharedSupports::Supports*>(object())->start_val[i];
}

forceinline void
SharedSupports::print(unsigned int i,unsigned int j) const {
  const SharedSupports::Supports* s =
    static_cast<SharedSupports::Supports*>(object());
  s->supports[s->row(i,j)].print();
}

forceinline unsigned int
SharedSupports::row(unsigned int var, int val) const {
  return static_cast<SharedSupports::Supports*>(object())->row(var,val);
}

forceinline
SharedSupports::~SharedSupports(void) {}
