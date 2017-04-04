#ifndef LinkedList_h
#define LinkedList_h

#include <iostream>
#include <string>
using namespace std;

struct Key {
  int x;
  int y;
  bool operator ==(const Key& k) {
    return x == k.x && y == k.y;
  }
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
  /// Construct empty list object
  LinkedList();
  /// Copy \a l
  LinkedList(const LinkedList& l);
  /// Insert \a item into the list
  void insert(Item* item);
  /// Remove the item with key \a key, return true upon success
  bool remove(Key key);
  /// Get pointer to item with key \a key
  Item* get(Key key) const;
  /// Get the length of the list.
  int length() const;
  /// Destructor
  ~LinkedList();
  /// Print the contents of the list
  void print() const;
};

#endif


