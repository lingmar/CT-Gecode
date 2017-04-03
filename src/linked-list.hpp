#ifndef LinkedList_h
#define LinkedList_h

#include <iostream>
#include <string>
using namespace std;

struct Key {
  int var;
  int val;
};

struct Item {
  Key key;
  int row;
  Item* next;
};

class LinkedList {
private:
  /// The head of the list
  Item* head;
  /// Number of items
  unsigned int sz;
public:
  /// Constructs empty list object
  LinkedList();
  /// Insert \a item into the list
  void insert(Item* item);
  /// Remove the item with key \a key, return true upon success
  bool remove(Key key);
  /// Get pointer to item with key \a key
  Item* get(Key key);
  /// Get the length of the list.
  int length();
  /// Destructor
  ~LinkedList();
};

#endif


