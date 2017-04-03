#ifndef HashTable_h
#define HashTable_h

#include "LinkedList.h"

class HashTable {
private:
  /// The table
  LinkedList* table;
  /// The length of the hash table
  unsigned int length;
  /// Returns an array location for a given item key.
  unsigned int hash(Key key);
public:
  /// Create hash table with length \a length
  HashTable(unisigned int length);
  /// Adds an item to the Hash Table.
  void insert(Item* newItem );
  /// Remove item with key \a key and return true upon success
  bool remove(Key key);
  /// Return a pointer to the item with key \a key
  Item* get(Key key);
  /// Get the length of the hash table 
  unsigned int length();
  /// Returns the number of Items in the Hash Table.
  unsigned int items();
  /// Destructor
  ~HashTable();
};

#endif
