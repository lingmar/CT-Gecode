#include "hash-table.hpp"
#include <gecode/kernel.hh>

#define PRIME_1 7
#define PRIME_2 11

forceinline
HashTable::HashTable(void) {}

forceinline
HashTable::HashTable(const HashTable& ht) :
  sz(ht.sz), table(ht.table) {}

forceinline void
HashTable::init(unsigned int _sz) {
  sz = _sz;
  table = heap.alloc<LinkedList>(sz);
}

forceinline unsigned int
HashTable::hash(Key key) {
  return (key.x * PRIME_1 + key.y * PRIME_2) % sz;
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

forceinline Item*
HashTable::get(Key key) {
    int index = hash(key);
    return table[index].get(key);
}

forceinline unsigned int
HashTable::length() {
    return sz;
}

forceinline unsigned int
HashTable::items() {
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
