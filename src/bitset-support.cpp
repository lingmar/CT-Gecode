#include <gecode/kernel.hh>
#include "bitset.hpp"

using namespace Gecode;

class SharedSupports : public SharedHandle {
protected:
  class Supports : public SharedHandle::Object {
  public:
    BitSet* supports;
    unsigned int* start_idx;
    unsigned int* start_val;
    unsigned int domsize;
    unsigned int nsupports;
    unsigned int vars;
    Supports(int domsize0, int nsupports0, int vars0)
      : domsize(domsize0), nsupports(nsupports0), vars(vars0) {
      supports = heap.alloc<BitSet>(domsize);
      start_idx = heap.alloc<unsigned int>(vars);
      start_val = heap.alloc<unsigned int>(vars);
    }
    Supports(const Supports& sio)
      : domsize(sio.domsize), nsupports(sio.nsupports), vars(sio.vars) {
      supports = heap.alloc<BitSet>(sio.domsize);
      for (int i = 0; i < domsize; i++) {
        // Initialise bit-sets
      }
      start_idx = heap.alloc<unsigned int>(sio.vars);
      start_val = heap.alloc<unsigned int>(sio.vars);
      for (int i = 0; i < vars; i++) {
        start_idx[i] = sio.start_idx[i];
        start_val[i] = sio.start_val[i];
      }
    }
    /// Initialise supports
    void init_supports(int nsupports0) {
      for (int i = 0; i < nsupports0; i++) {
        supports[i].init(heap,nsupports,false);
      }
    }
    /// row number
    unsigned int row(int var, int val) const {
      return start_idx[var] + val - start_val[var];
    }
    /// Copy
    virtual Object* copy(void) const {
      return new Supports(*this);
    }
    virtual ~Supports(void) {}
  };
public:
  SharedSupports(int domsize, int nsupports, int vars) 
    : SharedHandle(new Supports(domsize,nsupports,vars)) {}
  SharedSupports(const SharedSupports& si)
    : SharedHandle(si) {}
  SharedSupports& operator =(const SharedSupports& si) {
    return static_cast<SharedSupports&>(SharedHandle::operator =(si));
  }
  BitSet& get(int i, int j) const {
    const Supports* s = static_cast<const Supports*>(object());
    return s->supports[s->row(i,j)];
  }
  void set(int i, int j, int n) {
    const Supports* s = static_cast<Supports*>(object());
    s->supports[s->row(i,j)].set(n);
  }
  // some inherited members
  void update(Space& home, bool share, SharedHandle& sh) {
    SharedHandle::update(home,share,sh);
  }
  ~SharedSupports(void) {}
};

