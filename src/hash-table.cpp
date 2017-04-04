#include "hash-table.hpp"
#include <gecode/kernel.hh>

#define PRIME_1 7
#define PRIME_2 11

#define TABLE_SIZE 137

forceinline
HashTable::HashTable(void) {}

forceinline
HashTable::HashTable(const HashTable& ht) :
  sz(ht.sz), table(ht.table) {
  //print();
}

forceinline void
HashTable::init(unsigned int _sz) {
  //printf("%d\n", _sz);
  //sz = _sz;
  sz = 137;
  table = heap.alloc<LinkedList>(sz);
}

forceinline unsigned int
HashTable::hash(Key key) const {
  unsigned int ret = key.x;
  ret *= 2654435761U;
  return (ret ^ key.y) % sz;
  //return (key.x * PRIME_1 + key.y * PRIME_2) % sz;
}

forceinline void
HashTable::insert(Item* item) {
    int index = hash(item->key);
    table[index].insert(item);
}

forceinline bool
HashTable::remove(Key key) {
  int index = hash(key);
  return table[index].remove(key);
}

forceinline int
HashTable::get(Key key) const {
  int index = hash(key);
  return table[index].get(key)->row;
}

forceinline unsigned int
HashTable::length() const {
    return sz;
}

forceinline unsigned int
HashTable::items() const {
    int count = 0;
    for (int i = 0; i < sz; i++) {
        count += table[i].length();
    }
    return count;
}

forceinline
HashTable::~HashTable()
{
    delete [] table;
}

forceinline void
HashTable::print() const {
  for (int i = 0; i < sz; i++) {
    cout << i << ": ";
    table[i].print();
  }
}
