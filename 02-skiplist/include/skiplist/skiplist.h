#ifndef __SKIPLIST_H
#define __SKIPLIST_H
#include <functional>
#include "node.h"
#include "iterator.h"
#include <stack>

template<class Key, class Value, size_t MAXHEIGHT, class Compare = std::less<Key>>
class SkipList {
private:
  DataNode<Key, Value> *pHead; // changed Node * to DataNote *
  DataNode<Key, Value> *pTail;

  IndexNode<Key, Value> *pTailIdx;
  IndexNode<Key, Value> *aHeadIdx[MAXHEIGHT];

public:
  /**
   * Creates new empty skiplist
   */
  SkipList() {
    std::srand(std::time(nullptr));

    pHead   = new DataNode<Key, Value>(nullptr, nullptr);
    pTail   = new DataNode<Key, Value>(nullptr, nullptr);
    pHead->next(pTail);

    Node<Key, Value> *prev = pHead;
    pTailIdx = new IndexNode<Key, Value>(pTail, pTail); // down = ptail, root = ptail
    for (int i=0; i < MAXHEIGHT; i++) {
      aHeadIdx[i] = new IndexNode<Key, Value>(prev, pHead); // down = prev, root = pHead
      aHeadIdx[i]->next(pTailIdx);
      prev = aHeadIdx[i];
    }
  }

  /**
   * Disable copy constructor
   */
  SkipList(const SkipList& that) = delete;


  /**
   * Destructor
   */
  virtual ~SkipList() {
    for (int i = MAXHEIGHT - 1; i >= 0; i--) {
      IndexNode<Key, Value> *node = aHeadIdx[i];
      while (node != pTailIdx) {
        IndexNode<Key, Value> *delNode = node;
        node = &node->next();
        delete delNode;
      }
    }
    delete pTailIdx;

    DataNode<Key, Value> *dataNode = &pHead->next();
    delete pHead;
    while(dataNode != pTail) {
      DataNode<Key, Value> *delDataNode = dataNode;
      dataNode = &dataNode->next();
      delete delDataNode;
    }
    delete pTail;
  }

  /**
   * Assign new value for the key. If a such key already has
   * assosiation then old value returns, otherwise nullptr
   *
   * @param key key to be assigned with value
   * @param value to be added
   * @return old value for the given key or nullptr
   */
  virtual Value* Put(const Key& key, Value& value) const {
    Value *old_value = Delete(key);

    PutIfAbsent(key, value);

    return old_value;
  };

  /**
   * Put value only if there is no assosiation with key in
   * the list and returns nullptr
   *
   * If there is an established assosiation with the key already
   * method doesn't do anything and returns existing value
   *
   * @param key key to be assigned with value
   * @param value to be added
   * @return existing value for the given key or nullptr
   */
  virtual Value* PutIfAbsent(const Key& key, Value& value) const {
    Value *get_value = Get(key);
    if (get_value != nullptr) {
      return get_value;
    }

    std::stack<IndexNode<Key, Value> *> ladder;
    int height = MAXHEIGHT - 1;
    IndexNode<Key, Value> *cur = aHeadIdx[height];
    while (height >= 0) {
      if (&cur->next() == pTailIdx || cur->next().key() > key) {
        ladder.push(cur);
        if (height != 0) {
          cur = dynamic_cast<IndexNode<Key, Value> *>(&cur->down());
        }
        height--;
      } else {
        cur = dynamic_cast<IndexNode<Key, Value> *>(&cur->next());
      }
    }
    DataNode<Key, Value> *data_cur = dynamic_cast<DataNode<Key, Value> *>(&cur->down());
    while(&data_cur->next() != pTail && data_cur->next().key() < key) {
      data_cur = &data_cur->next();
    }
    InsertNext(data_cur, ladder, key, value);

    return nullptr;
  };

  void InsertNext(DataNode<Key, Value> *cur, std::stack<IndexNode<Key, Value> *> ladder, const Key &key, Value &value) const {
    DataNode<Key, Value> *new_node = new DataNode<Key, Value>(&key, &value); //memory
    cur->next(new_node);
    new_node->next(pTail);

    Node<Key, Value> *down = new_node;
    for (int i = 0; i < MAXHEIGHT; i++) { //check actual elemcount in stack
      double r = ((double) std::rand() / (RAND_MAX));
      if (r > 0.5) {
        // std::cout << "breaking on height " << i << std::endl;
        break;
      }

      IndexNode<Key, Value> *step = ladder.top();
      IndexNode<Key, Value> *new_index_node = new IndexNode<Key, Value>(down, new_node, &step->next());
      step->next(new_index_node);
      down = new_index_node;
      ladder.pop();
    }
  }

  /**
   * Returns value assigned for the given key or nullptr
   * if there is no established assosiation with the given key
   *
   * @param key to find
   * @return value assosiated with given key or nullptr
   */
  virtual Value* Get(const Key& key) const {
    int height = MAXHEIGHT - 1;
    IndexNode<Key, Value> *cur = aHeadIdx[height];
    while (height >= 0) {
      if (&cur->next() == pTailIdx || cur->next().key() > key) {
        if (height != 0) {
          cur = dynamic_cast<IndexNode<Key, Value> *>(&cur->down());
        }
        height--;
      } else if (cur->next().key() == key) {
        return &cur->next().value();
      } else {
        cur = dynamic_cast<IndexNode<Key, Value> *>(&cur->next());
      }
    }
    DataNode<Key, Value> *data_cur = dynamic_cast<DataNode<Key, Value> *>(&cur->down());
    while (&data_cur->next() != pTail && data_cur->next().key() < key) {
      data_cur = &data_cur->next();
    }
    if (&data_cur->next() == pTail || data_cur->next().key() != key) {
      return nullptr;
    } else {
      return &data_cur->next().value();
    }
  };

  /**
   * Remove given key from the skiplist and returns value
   * it has or nullptr in case if key wasn't assosiated with
   * any value
   *
   * @param key to be added
   * @return value for the removed key or nullptr
   */
  virtual Value* Delete(const Key& key) const {
    int height = MAXHEIGHT - 1;
    IndexNode<Key, Value> *cur = aHeadIdx[height];
    while (height >= 0) {
      if (&cur->next() == pTailIdx || cur->next().key() > key) {
        if (height != 0) {
          cur = dynamic_cast<IndexNode<Key, Value> *>(&cur->down());
        }
        height--;
      } else if (cur->next().key() == key) {
        // remove next indexNode all the way down
        IndexNode<Key, Value> *delNode = &cur->next();
        Value *value = &delNode->value();
        cur->next(&delNode->next());
        delete delNode;
        Delete(key);
        return value;
      } else {
        cur = dynamic_cast<IndexNode<Key, Value> *>(&cur->next());
      }
    }
    DataNode<Key, Value> *data_cur = dynamic_cast<DataNode<Key, Value> *>(&cur->down());
    while (&data_cur->next() != pTail && data_cur->next().key() < key) {
      data_cur = &data_cur->next();
    }
    if (&data_cur->next() == pTail || data_cur->next().key() != key) {
      return nullptr;
    } else {
      DataNode<Key, Value> *delDataNode = &data_cur->next();
      Value *value = &delDataNode->value();
      data_cur->next(&delDataNode->next());
      delete delDataNode;
      return value;
    }
  };

  /**
   * Same as Get
   */
  virtual const Value* operator[](const Key& key) {
    return Get(key);
  };

  void cout_dump() {
    for (int i = MAXHEIGHT - 1; i >= 0; i--) {
      std::cout << "height " << i << " ";
      IndexNode<Key, Value> *node = &aHeadIdx[i]->next();
      while (node != pTailIdx) {
        std::cout << "{key : " << node->key() << ", value " << node->value() << "} ";
        node = &node->next();
      }
      std::cout << std::endl;
    }
    std::cout << "ground ";
    DataNode<Key, Value> *dataNode = &pHead->next();
    while(dataNode != pTail) {
      std::cout << "{key : " << dataNode->key() << ", value " << dataNode->value() << "} ";
      dataNode = &dataNode->next();
    }
    std::cout << std::endl;
  }

  /**
   * Return iterator onto very first key in the skiplist
   */
  virtual Iterator<Key, Value> cbegin() const {
    DataNode<Key, Value> &firstNode = pHead->next();
    return Iterator<Key,Value>(&firstNode);
  };

  /**
   * Returns iterator to the first key that is greater or equals to
   * the given key
   */
  virtual Iterator<Key, Value> cfind(const Key &key) const {
        int height = MAXHEIGHT - 1;
    IndexNode<Key, Value> *cur = aHeadIdx[height];
    while (height >= 0) {
      if (&cur->next() == pTailIdx || cur->next().key() > key) {
        if (height != 0) {
          cur = dynamic_cast<IndexNode<Key, Value> *>(&cur->down());
        }
        height--;
      } else if (cur->next().key() == key) {
        return Iterator<Key, Value>(&cur->next().root());
      } else {
        cur = dynamic_cast<IndexNode<Key, Value> *>(&cur->next());
      }
    }
    DataNode<Key, Value> *data_cur = dynamic_cast<DataNode<Key, Value> *>(&cur->down());
    while (&data_cur->next() != pTail && data_cur->next().key() < key) {
      data_cur = &data_cur->next();
    }
    return Iterator<Key, Value>(&data_cur->next());
  };

  /**
   * Returns iterator on the skiplist tail
   */
  virtual Iterator<Key, Value> cend() const {
    return Iterator<Key,Value>(pTail);
  };
};
#endif // __SKIPLIST_H
