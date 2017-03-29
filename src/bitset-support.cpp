#include <gecode/kernel.hh>
#include "bitset.hpp"

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
    void allocate_supports(int d,int v,int n);
    /// Row number for variable \a var and value \a val
    unsigned int row(int var, int val) const;
    /// Copy function
    virtual Object* copy(void) const;
    /// Destructor
    virtual ~Supports(void);
  };
public:
  /// Default constructor (yields empty set)
  SharedSupports(void);
  /// Copy \a si
  SharedSupports(const SharedSupports& si);
  /// Assignment operator
  SharedSupports& operator =(const SharedSupports& si);
  /// Get bit set for variable i, value j
  BitSet& get(int i, int j) const;
  /// Allocate supports for \a d rows, \a variables and \a n bits
  void init_supports(int d, int v, int n);
  /// Set start index for variable \a i to \a idx
  void set_start_idx(unsigned int i, unsigned int idx);
  /// Set start value for variable \a i to \a val
  void set_start_val(unsigned int i, unsigned int val);
  /// Print bit set for variable \a i and value \a
  void print(int i,int j);
  /// Destructor
  ~SharedSupports(void);
};

forceinline
SharedSupports::Supports::Supports(void)
  : supports(NULL), start_idx(NULL), domsize(0),
    nsupports(0), vars(0) {
}

forceinline
SharedSupports::Supports::Supports(const Supports& sio)
  : domsize(sio.domsize), nsupports(sio.nsupports), vars(sio.vars) {
  //std::cout << "copy constructor" << std::endl;
  supports = heap.alloc<BitSet>(sio.domsize);
  for (int i = 0; i < domsize; i++) {
    supports[i].init(heap,nsupports,false);
    supports[i].copy(nsupports,sio.supports[i]);
  }
  start_idx = heap.alloc<unsigned int>(sio.vars);
  start_val = heap.alloc<unsigned int>(sio.vars);
  for (int i = 0; i < vars; i++) {
    start_idx[i] = sio.start_idx[i];
    start_val[i] = sio.start_val[i];
  }
}

forceinline void
SharedSupports::Supports::allocate_supports(int d,int v,int n) {
  //std::cout << "init_supports" << std::endl;
  domsize = d;
  vars = v;
  nsupports = n;
  supports = heap.alloc<BitSet>(domsize);
  start_idx = heap.alloc<unsigned int>(vars);
  start_val = heap.alloc<unsigned int>(vars);
  for (int i = 0; i < d; i++) {
    supports[i].init(heap,nsupports,false);
  }
}

forceinline unsigned int
SharedSupports::Supports::row(int var, int val) const {
  return start_idx[var] + val - start_val[var];
}

forceinline SharedSupports::Object*
SharedSupports::Supports::copy(void) const {
  return new Supports(*this);
}

forceinline
SharedSupports::Supports::~Supports(void) {}

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
SharedSupports::get(int i, int j) const {
  const SharedSupports::Supports* s =
    static_cast<const SharedSupports::Supports*>(object());
  return s->supports[s->row(i,j)];
}

forceinline void
SharedSupports::init_supports(int d, int v, int n) {
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

forceinline void
SharedSupports::print(int i,int j) {
  const SharedSupports::Supports* s =
    static_cast<SharedSupports::Supports*>(object());
  s->supports[s->row(i,j)].print();
}

forceinline
SharedSupports::~SharedSupports(void) {}
