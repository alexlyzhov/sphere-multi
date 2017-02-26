#include "gtest/gtest.h"
#include <string>
#include <skiplist/skiplist.h>

using namespace std;

SkipList<int, string, 8> sk;

TEST(SkipListTest, Empty) {
  ASSERT_EQ(nullptr, sk.Get(100));
  ASSERT_EQ(sk.cend(), sk.cbegin())  << "Begin iterator fails";
  ASSERT_EQ(sk.cend(), sk.cfind(10)) << "Find iterator fails";
}

TEST(SkipListTest, SimplePut) {
  SkipList<int, string, 8> sk;

  string str1 = std::string("delete this string");
  const std::string *old = sk.Put(666, str1);
  ASSERT_NE(sk.cbegin(), sk.cend());

  // sk.cout_dump();
  sk.Delete(666);
  // sk.cout_dump();
  ASSERT_EQ(sk.cbegin(), sk.cend());

  string str = std::string("test");
  const std::string *pOld = sk.Put(10, str);
  ASSERT_EQ(nullptr, pOld);

  Iterator<int, std::string> it1 = sk.cfind(9);
  ASSERT_NE(it1, sk.cend());

  pOld = sk[10];
  ASSERT_NE(nullptr, pOld)         << "Value found";
  ASSERT_EQ(string("test"), *pOld) << "Value is correct";

  pOld = sk.Get(10);
  ASSERT_NE(nullptr, pOld)         << "Value found";
  ASSERT_EQ(string("test"), *pOld) << "Value is correct";

  Iterator<int,std::string> it = sk.cbegin();
  ASSERT_NE(sk.cend(), it)              << "Iterator is not empty";
  ASSERT_EQ(10, it.key())               << "Iterator key is correct";
  ASSERT_EQ(string("test"), it.value()) << "Iterator value is correct";
  ASSERT_EQ(string("test"), *it)        << "Iterator value is correct";
}
