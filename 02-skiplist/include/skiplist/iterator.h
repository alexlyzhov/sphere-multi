#ifndef __ITERATOR_H
#define __ITERATOR_H
#include <cassert>
#include "node.h"

/**
 * Skiplist const iterator
 */
template<class Key, class Value>
class Iterator {
private:
  Node<Key, Value> *pCurrent;

public:
  Iterator(Node<Key,Value> *p) : pCurrent(p) {}
  virtual ~Iterator() {}

  virtual const Key& key() const {
    assert(pCurrent != nullptr);
    return pCurrent->key();
  };

  virtual const Value& value() const {
    assert(pCurrent != nullptr);
    return pCurrent->value();
  };

  virtual const Value& operator*() {
    assert(pCurrent != nullptr);
    return pCurrent->value();
  };

  virtual const Value& operator->() {
    assert(pCurrent != nullptr);
    return pCurrent->value();
  };

  virtual bool operator==(const Iterator &another) const {
    return pCurrent == another.pCurrent;
  };

  virtual bool operator!=(const Iterator &another) const {
    return !(*this == another);
  };


  virtual Iterator<Key, Value>& operator=(const Iterator &another) {
    pCurrent = another.pCurrent;
    return *this;
  };

  virtual Iterator<Key, Value>& operator++() {
    pCurrent = &pCurrent->next();
    return *this;
  };

  virtual Iterator<Key, Value> operator++(int num) {
    Iterator<Key, Value> old = *this;
    pCurrent = &pCurrent->next();
    return old;
  };
};

#endif // __ITERATOR_H
