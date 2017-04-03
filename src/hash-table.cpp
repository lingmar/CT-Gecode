#include "HashTable.h"
#include <gecode/kernel.hh>

#define PRIME_1 7
#define PRIME_2 11

forceinline
HashTable::HashTable(unsigned int _sz)
  : sz(_sz) {
  table = heap.alloc<LinkedList>(_sz);
}

forceinline int
HashTable::hash(Key key) {
  return key->(var * PRIME_1 + val * PRIME_2) % sz;
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
    delete [] array;
}
