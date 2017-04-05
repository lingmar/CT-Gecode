#include "linked-list.hpp"
#include <gecode/kernel.hh>

using namespace Gecode;
using namespace std;

forceinline
LinkedList::LinkedList() {
  head = heap.alloc<Item>(1);
  head->next = NULL;
  sz = 0;
}

forceinline
LinkedList::LinkedList(const LinkedList& l)
  : head(l.head), sz(l.sz) {
  Item* p = head;
  Item* q = l.head;
  while (p) {
    p->next = heap.alloc<Item>(1);
    p = p->next;
    q = q->next;
    p->key = q->key;
    p->row = q->row;
    p->next = q->next;
  }
} 

forceinline void
LinkedList::insert(Item* item) {
  if (!head->next) {
    head->next = item;
    sz++;
    return;
  }
  Item* p = head;
  Item* q = head;
  while (q) {
    p = q;
    q = p->next;
  }
  p->next = item;
  item->next = NULL;
  sz++;
}

forceinline bool
LinkedList::remove(Key key) {
  if (!head->next) return false;
  Item * p = head;
  Item * q = head;
  while (q) {
    if (q->key == key) {
      p->next = q->next;
      delete q;
      sz--;
      return true;
    }
    p = q;
    q = p->next;
  }
  return false;
}

forceinline Item*
LinkedList::get(Key key) const {
  Item * p = head;
  Item * q = head;
  while (q) {
    p = q;
   if ((p != head) && (p->key == key))
      return p;
    q = p->next;
  }
  return NULL;
}

forceinline bool
LinkedList::set(Key key, int val) {
  Item * p = head;
  Item * q = head;
  while (q) {
    p = q;
    if ((p != head) && (p->key == key)) {
      p->row = val;
      return true;
    }
    q = p->next;
  }
  return false;
}

forceinline int
LinkedList::length() const {
  return sz;
}

forceinline
LinkedList::~LinkedList() {
  Item * p = head;
  Item * q = head;
  while (q) {
    p = q;
    q = p->next;
    if (q) delete p;
  }
}

forceinline void
LinkedList::print() const {
  Item* p = head;
  cout << "{";
  while (p->next) {
    p = p->next;
    printf("{Key: {%d,%d}, Row: %d} ", p->key.x, p->key.y, p->row);
  }
  cout << "}\n";
}
