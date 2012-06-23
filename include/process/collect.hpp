#ifndef __PROCESS_COLLECT_HPP__
#define __PROCESS_COLLECT_HPP__

#include <assert.h>

#include <list>

#include <process/defer.hpp>
#include <process/delay.hpp>
#include <process/future.hpp>
#include <process/option.hpp>
#include <process/process.hpp>
#include <process/timeout.hpp>

namespace process {

// Waits on each future in the specified set and returns the set of
// resulting values. If any future is discarded then the result will
// be a failure. Likewise, if any future fails than the result future
// will be a failure.
template <typename T>
Future<std::list<T> > collect(
    std::list<Future<T> >& futures,
    const Option<Timeout>& timeout = Option<Timeout>::none());


namespace internal {

template <typename T>
class CollectProcess : public Process<CollectProcess<T> >
{
public:
  CollectProcess(
      const std::list<Future<T> >& _futures,
      const Option<Timeout>& _timeout,
      Promise<std::list<T> >* _promise)
    : futures(_futures),
      timeout(_timeout),
      promise(_promise) {}

  virtual ~CollectProcess()
  {
    delete promise;
  }

  virtual void initialize()
  {
    // Stop this nonsense if nobody cares.
    promise->future().onDiscarded(defer(this, &CollectProcess::discarded));

    // Only wait as long as requested.
    if (timeout.isSome()) {
      delay(timeout.get().remaining(), this, &CollectProcess::timedout);
    }

    typename std::list<Future<T> >::const_iterator iterator;
    for (iterator = futures.begin(); iterator != futures.end(); ++iterator) {
      const Future<T>& future = *iterator;
      future.onAny(defer(this, &CollectProcess::waited, future));
    }
  }

private:
  void discarded()
  {
    terminate(this);
  }

  void timedout()
  {
    promise->fail("Collect failed: timed out");
    terminate(this);
  }

  void waited(const Future<T>& future)
  {
    if (future.isFailed()) {
      promise->fail("Collect failed: " + future.failure());
    } else if (future.isDiscarded()) {
      promise->fail("Collect failed: future discarded");
    } else {
      assert(future.isReady());
      values.push_back(future.get());
      if (futures.size() == values.size()) {
        promise->set(values);
        terminate(this);
      }
    }
  }

  const std::list<Future<T> > futures;
  const Option<Timeout> timeout;
  Promise<std::list<T> >* promise;
  std::list<T> values;
};

} // namespace internal {


template <typename T>
inline Future<std::list<T> > collect(
    std::list<Future<T> >& futures,
    const Option<Timeout>& timeout)
{
  Promise<std::list<T> >* promise = new Promise<std::list<T> >();
  spawn(new internal::CollectProcess<T>(futures, timeout, promise), true);
  return promise->future();
}

} // namespace process {

#endif // __PROCESS_COLLECT_HPP__
