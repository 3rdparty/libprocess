#ifndef __PROCESS_GTEST_HPP__
#define __PROCESS_GTEST_HPP__

#include <gtest/gtest.h>

#include <string>

#include <process/clock.hpp>
#include <process/future.hpp>
#include <process/http.hpp>

#include <stout/duration.hpp>
#include <stout/option.hpp>

namespace process {

// A simple test event listener that makes sure to resume the clock
// after each test even if the previous test had a partial result
// (i.e., an ASSERT_* failed).
class ClockTestEventListener : public ::testing::EmptyTestEventListener
{
public:
  // Returns the singleton instance of the listener.
  static ClockTestEventListener* instance()
  {
    static ClockTestEventListener* listener = new ClockTestEventListener();
    return listener;
  }

  virtual void OnTestEnd(const ::testing::TestInfo&)
  {
    if (process::Clock::paused()) {
      process::Clock::resume();
    }
  }
private:
  ClockTestEventListener() {}
};

} // namespace process {

template <typename T>
::testing::AssertionResult AwaitAssertReady(
    const char* expr,
    const char*, // Unused string representation of 'duration'.
    const process::Future<T>& actual,
    const Duration& duration)
{
  if (!actual.await(duration)) {
    return ::testing::AssertionFailure()
      << "Failed to wait " << duration << " for " << expr;
  } else if (actual.isDiscarded()) {
    return ::testing::AssertionFailure()
      << expr << " was discarded";
  } else if (actual.isFailed()) {
    return ::testing::AssertionFailure()
      << "(" << expr << ").failure(): " << actual.failure();
  }

  return ::testing::AssertionSuccess();
}


template <typename T>
::testing::AssertionResult AwaitAssertFailed(
    const char* expr,
    const char*, // Unused string representation of 'duration'.
    const process::Future<T>& actual,
    const Duration& duration)
{
  if (!actual.await(duration)) {
    return ::testing::AssertionFailure()
      << "Failed to wait " << duration << " for " << expr;
  } else if (actual.isDiscarded()) {
    return ::testing::AssertionFailure()
      << expr << " was discarded";
  } else if (actual.isReady()) {
    return ::testing::AssertionFailure()
      << expr << " is ready (" << ::testing::PrintToString(actual.get()) << ")";
  }

  return ::testing::AssertionSuccess();
}


template <typename T>
::testing::AssertionResult AwaitAssertDiscarded(
    const char* expr,
    const char*, // Unused string representation of 'duration'.
    const process::Future<T>& actual,
    const Duration& duration)
{
  if (!actual.await(duration)) {
    return ::testing::AssertionFailure()
      << "Failed to wait " << duration << " for " << expr;
  } else if (actual.isFailed()) {
    return ::testing::AssertionFailure()
      << "(" << expr << ").failure(): " << actual.failure();
  } else if (actual.isReady()) {
    return ::testing::AssertionFailure()
      << expr << " is ready (" << ::testing::PrintToString(actual.get()) << ")";
  }

  return ::testing::AssertionSuccess();
}


template <typename T1, typename T2>
::testing::AssertionResult AwaitAssertEq(
    const char* expectedExpr,
    const char* actualExpr,
    const char* durationExpr,
    const T1& expected,
    const process::Future<T2>& actual,
    const Duration& duration)
{
  const ::testing::AssertionResult result =
    AwaitAssertReady(actualExpr, durationExpr, actual, duration);

  if (result) {
    if (expected == actual.get()) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure()
        << "Value of: (" << actualExpr << ").get()\n"
        << "  Actual: " << ::testing::PrintToString(actual.get()) << "\n"
        << "Expected: " << expectedExpr << "\n"
        << "Which is: " << ::testing::PrintToString(expected);
    }
  }

  return result;
}


#define AWAIT_ASSERT_READY_FOR(actual, duration)                \
  ASSERT_PRED_FORMAT2(AwaitAssertReady, actual, duration)


#define AWAIT_ASSERT_READY(actual)              \
  AWAIT_ASSERT_READY_FOR(actual, Seconds(5))


#define AWAIT_READY_FOR(actual, duration)       \
  AWAIT_ASSERT_READY_FOR(actual, duration)


#define AWAIT_READY(actual)                     \
  AWAIT_ASSERT_READY(actual)


#define AWAIT_EXPECT_READY_FOR(actual, duration)                \
  EXPECT_PRED_FORMAT2(AwaitAssertReady, actual, duration)


#define AWAIT_EXPECT_READY(actual)              \
  AWAIT_EXPECT_READY_FOR(actual, Seconds(5))


#define AWAIT_ASSERT_FAILED_FOR(actual, duration)               \
  ASSERT_PRED_FORMAT2(AwaitAssertFailed, actual, duration)


#define AWAIT_ASSERT_FAILED(actual)             \
  AWAIT_ASSERT_FAILED_FOR(actual, Seconds(5))


#define AWAIT_FAILED_FOR(actual, duration)       \
  AWAIT_ASSERT_FAILED_FOR(actual, duration)


#define AWAIT_FAILED(actual)                    \
  AWAIT_ASSERT_FAILED(actual)


#define AWAIT_EXPECT_FAILED_FOR(actual, duration)               \
  EXPECT_PRED_FORMAT2(AwaitAssertFailed, actual, duration)


#define AWAIT_EXPECT_FAILED(actual)             \
  AWAIT_EXPECT_FAILED_FOR(actual, Seconds(5))


#define AWAIT_ASSERT_DISCARDED_FOR(actual, duration)            \
  ASSERT_PRED_FORMAT2(AwaitAssertDiscarded, actual, duration)


#define AWAIT_ASSERT_DISCARDED(actual)                  \
  AWAIT_ASSERT_DISCARDED_FOR(actual, Seconds(5))


#define AWAIT_DISCARDED_FOR(actual, duration)       \
  AWAIT_ASSERT_DISCARDED_FOR(actual, duration)


#define AWAIT_DISCARDED(actual)                 \
  AWAIT_ASSERT_DISCARDED(actual)


#define AWAIT_EXPECT_DISCARDED_FOR(actual, duration)            \
  EXPECT_PRED_FORMAT2(AwaitAssertDiscarded, actual, duration)


#define AWAIT_EXPECT_DISCARDED(actual)                  \
  AWAIT_EXPECT_DISCARDED_FOR(actual, Seconds(5))


#define AWAIT_ASSERT_EQ_FOR(expected, actual, duration)                 \
  ASSERT_PRED_FORMAT3(AwaitAssertEq, expected, actual, duration)


#define AWAIT_ASSERT_EQ(expected, actual)       \
  AWAIT_ASSERT_EQ_FOR(expected, actual, Seconds(5))


#define AWAIT_EQ(expected, actual)              \
  AWAIT_ASSERT_EQ(expected, actual)


#define AWAIT_EXPECT_EQ_FOR(expected, actual, duration)                 \
  EXPECT_PRED_FORMAT3(AwaitAssertEq, expected, actual, duration)


#define AWAIT_EXPECT_EQ(expected, actual)               \
  AWAIT_EXPECT_EQ_FOR(expected, actual, Seconds(5))


inline ::testing::AssertionResult AwaitAssertResponseStatusEq(
    const char* expectedExpr,
    const char* actualExpr,
    const char* durationExpr,
    const std::string& expected,
    const process::Future<process::http::Response>& actual,
    const Duration& duration)
{
  const ::testing::AssertionResult result =
    AwaitAssertReady(actualExpr, durationExpr, actual, duration);

  if (result) {
    if (expected == actual.get().status) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure()
        << "Value of: (" << actualExpr << ").get().status\n"
        << "  Actual: " << ::testing::PrintToString(actual.get().status) << "\n"
        << "Expected: " << expectedExpr << "\n"
        << "Which is: " << ::testing::PrintToString(expected);
    }
  }

  return result;
}


#define AWAIT_EXPECT_RESPONSE_STATUS_EQ_FOR(expected, actual, duration) \
  EXPECT_PRED_FORMAT3(AwaitAssertResponseStatusEq, expected, actual, duration)


#define AWAIT_EXPECT_RESPONSE_STATUS_EQ(expected, actual)               \
  AWAIT_EXPECT_RESPONSE_STATUS_EQ_FOR(expected, actual, Seconds(5))


inline ::testing::AssertionResult AwaitAssertResponseBodyEq(
    const char* expectedExpr,
    const char* actualExpr,
    const char* durationExpr,
    const std::string& expected,
    const process::Future<process::http::Response>& actual,
    const Duration& duration)
{
  const ::testing::AssertionResult result =
    AwaitAssertReady(actualExpr, durationExpr, actual, duration);

  if (result) {
    if (expected == actual.get().body) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure()
        << "Value of: (" << actualExpr << ").get().body\n"
        << "  Actual: " << ::testing::PrintToString(actual.get().body) << "\n"
        << "Expected: " << expectedExpr << "\n"
        << "Which is: " << ::testing::PrintToString(expected);
    }
  }

  return result;
}


#define AWAIT_EXPECT_RESPONSE_BODY_EQ_FOR(expected, actual, duration)   \
  EXPECT_PRED_FORMAT3(AwaitAssertResponseBodyEq, expected, actual, duration)


#define AWAIT_EXPECT_RESPONSE_BODY_EQ(expected, actual)                 \
  AWAIT_EXPECT_RESPONSE_BODY_EQ_FOR(expected, actual, Seconds(5))


inline ::testing::AssertionResult AwaitAssertResponseHeaderEq(
    const char* expectedExpr,
    const char* keyExpr,
    const char* actualExpr,
    const char* durationExpr,
    const std::string& expected,
    const std::string& key,
    const process::Future<process::http::Response>& actual,
    const Duration& duration)
{
  const ::testing::AssertionResult result =
    AwaitAssertReady(actualExpr, durationExpr, actual, duration);

  if (result) {
    const Option<std::string> value = actual.get().headers.get(key);
    if (value.isNone()) {
      return ::testing::AssertionFailure()
        << "Response does not contain header '" << key << "'";
    } else if (expected == value.get()) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure()
        << "Value of: (" << actualExpr << ").get().headers[" << keyExpr << "]\n"
        << "  Actual: " << ::testing::PrintToString(value.get()) << "\n"
        << "Expected: " << expectedExpr << "\n"
        << "Which is: " << ::testing::PrintToString(expected);
    }
  }

  return result;
}


#define AWAIT_EXPECT_RESPONSE_HEADER_EQ_FOR(expected, key, actual, duration) \
  EXPECT_PRED_FORMAT4(AwaitAssertResponseHeaderEq, expected, key, actual, duration)


#define AWAIT_EXPECT_RESPONSE_HEADER_EQ(expected, key, actual)          \
  AWAIT_EXPECT_RESPONSE_HEADER_EQ_FOR(expected, key, actual, Seconds(5))

#endif // __PROCESS_GTEST_HPP__
