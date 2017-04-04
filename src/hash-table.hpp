#ifndef HashTable_h
#define HashTable_h

#include "linked-list.hpp"

class HashTable {
private:
  /// The table
  LinkedList* table;
  /// The length of the hash table
  unsigned int sz;
public:
  /// Get the index for key \a key
  unsigned int hash(Key key);
  /// Default constructor (yields empty hash table)
  HashTable(void);
  /// Copy \a ht
  HashTable(const HashTable& ht);
  /// Initialise a hash table with length \a sz (only after default constructor)
  void init(unsigned int length);
  /// Adds an item to the Hash Table.
  void insert(Item* newItem );
  /// Remove item with key \a key and return true upon success
  bool remove(Key key);
  /// Return a pointer to the item with key \a key
  Item* get(Key key);
  /// Get the length of the hash table 
  unsigned int length();
  /// Returns the number of Items in the Hash Table
  unsigned int items();
  /// Destructor
  ~HashTable();
};

#endif
