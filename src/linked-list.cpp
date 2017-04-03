#include "LinkedList.h"
#include <gecode/kernel.hh>

forceinline
LinkedList::LinkedList() {
  head = heap.alloc<Item>(1);
  head->next = NULL;
  sz = 0;
}

forceinline void
LinkedList::insert(Item* item) {
  if (!head->next)
    {
      head->next = item;
      sz++;
      return;
    }
  Item* p = head;
  Item* q = head;
  while (q)
    {
      p = q;
      q = p->next;
    }
  p->next = item;
  item->next = NULL;
  sz++;
}

forceinline bool
LinkedList::remove(Key key) {
  if (!head -> next) return false;
  Item * p = head;
  Item * q = head;
  while (q)
    {
      if (q -> key == itemKey)
        {
          p -> next = q -> next;
          delete q;
          sz--;
          return true;
        }
      p = q;
      q = p -> next;
    }
  return false;
}

forceinline Item*
LinkedList::get(Key key) {
  Item * p = head;
  Item * q = head;
  while (q)
    {
      p = q;
      if ((p != head) && (p -> key == itemKey))
        return p;
      q = p -> next;
    }
  return NULL;
}

forceinline int
LinkedList::length() {
  return sz;
}

forceinline
LinkedList::~LinkedList() {
  Item * p = head;
  Item * q = head;
  while (q)
    {
      p = q;
      q = p -> next;
      if (q) delete p;
    }
}
