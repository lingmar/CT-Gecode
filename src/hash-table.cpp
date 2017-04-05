#include <iostream>
#include <fstream>
#include <string>

#include "hash-table.hpp"
#include <gecode/kernel.hh>
#include "primes.hpp"

#define PRIME_1 7
#define PRIME_2 11

#define TABLE_SIZE 137

int test[3] = {1,2,3};

forceinline
HashTable::HashTable(void) {}

forceinline
HashTable::HashTable(const HashTable& ht) :
  sz(ht.sz), table(ht.table) {
}

forceinline void
HashTable::init(unsigned int _sz) {
  sz = _sz;
  table = heap.alloc<LinkedList>(sz);
}

forceinline unsigned int
HashTable::hash(Key key) const {
  //unsigned int ret = key.x;
  //ret *= UINT32_C(2654435761); // Knuth's hashing
  return (key.x ^ key.y) % sz;
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

forceinline bool
HashTable::set(Key key,int val) {
  int index = hash(key);
  return table[index].set(key,val);
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
HashTable::~HashTable() {
    delete [] table;
}

forceinline void
HashTable::print() const {
  for (int i = 0; i < sz; i++) {
    cout << i << ": ";
    table[i].print();
  }
}

forceinline unsigned int
HashTable::closest_prime(unsigned int n) {
  for (int i = 0; i < NPRIMES; i++) {
    if (primes[i] > n) {
      return primes[i];
    }
  }
  return primes[NPRIMES - 1];
}
