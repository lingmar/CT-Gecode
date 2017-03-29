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

    Supports(void)
      : supports(NULL), start_idx(NULL), domsize(0),
        nsupports(0), vars(0) {
    }
    
    Supports(int domsize0, int nsupports0, int vars0)
      : domsize(domsize0), nsupports(nsupports0), vars(vars0) {
      std::cout << "normal consturctor" << std::endl;
      supports = heap.alloc<BitSet>(domsize);
      start_idx = heap.alloc<unsigned int>(vars);
      start_val = heap.alloc<unsigned int>(vars);
    }
    Supports(const Supports& sio)
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
    /// Initialise supports
    void init_supports(int d,int v,int n) {
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
  SharedSupports(void) : SharedHandle(new Supports()) {}
  SharedSupports(int domsize, int nsupports, int vars) 
    : SharedHandle(new Supports(domsize,nsupports,vars)) {}
  SharedSupports(const SharedSupports& si)
    : SharedHandle(si) {}
  SharedSupports& operator =(const SharedSupports& si) {
    return static_cast<SharedSupports&>(SharedHandle::operator =(si));
  }
  BitSet& get(int i, int j) const {
    const Supports* s = static_cast<const Supports*>(object());
    // std::cout << "(i,j)=" << i << "," << j << std::endl;
    // std::cout << "row=" << s->row(i,j) << std::endl;
    // std::cout << s->supports << std::endl;
    //s->supports[0].print();
    return s->supports[s->row(i,j)];
  }
  void set(int i, int j, int n) {
    std::cout << n << std::endl;
    const Supports* s = static_cast<Supports*>(object());
    s->supports[s->row(i,j)].set(n);
  }
  // some inherited members
  void update(Space& home, bool share, SharedHandle& sh) {
    SharedHandle::update(home,share,sh);
  }
  void init_supports(int d, int v, int n) {
    static_cast<Supports*>(object())->init_supports(d,v,n);
  }
  void set_start_idx(unsigned int i, unsigned int idx) {
    static_cast<Supports*>(object())->start_idx[i] = idx;
  }
  void set_start_val(unsigned int i, unsigned int val) {
    static_cast<Supports*>(object())->start_val[i] = val;
  }
  void print(int i,int j) {
    const Supports* s = static_cast<Supports*>(object());
    s->supports[s->row(i,j)].print();
  }
  ~SharedSupports(void) {}
};

