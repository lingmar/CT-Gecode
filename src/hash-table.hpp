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
  /// Default constructor (yields empty hash table)
  HashTable(void);
  /// Copy \a ht
  HashTable(const HashTable& ht);
  /// Get the index for key \a key
  unsigned int hash(Key key) const;
  /// Initialise a hash table with length \a sz (only after default constructor)
  void init(unsigned int length);
  /// Adds an item to the Hash Table.
  void insert(Item* newItem );
  /// Remove item with key \a key and return true upon success
  bool remove(Key key);
  /// Get the value of the item with key \a key
  int get(Key key) const;
  /// Get the length of the hash table 
  unsigned int length() const;
  /// Returns the number of Items in the Hash Table
  unsigned int items() const;
  /// Destructor
  ~HashTable();
  /// Print the contents of the table
  void print() const;
  /// Find the cloesest prime number larger than \a n
  static unsigned int closest_prime(unsigned int n);
};

#endif
