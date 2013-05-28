#include <gtest/gtest.h>

#include <string>

#include <stout/thread.hpp>

TEST(Thread, local)
{
  ThreadLocal<std::string>* _s_ = new ThreadLocal<std::string>();

  std::string* s = new std::string();

  ASSERT_TRUE(*(_s_) == NULL);

  (*_s_) = s;

  ASSERT_TRUE(*(_s_) == s);
  ASSERT_FALSE(*(_s_) == NULL);

  (*_s_) = NULL;

  ASSERT_TRUE(*(_s_) == NULL);

  delete s;
  delete _s_;
}
