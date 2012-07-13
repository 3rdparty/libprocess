#include <arpa/inet.h>

#include <gmock/gmock.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <string>
#include <sstream>

#include <process/collect.hpp>
#include <process/clock.hpp>
#include <process/defer.hpp>
#include <process/delay.hpp>
#include <process/dispatch.hpp>
#include <process/executor.hpp>
#include <process/filter.hpp>
#include <process/future.hpp>
#include <process/gc.hpp>
#include <process/io.hpp>
#include <process/process.hpp>
#include <process/run.hpp>
#include <process/thread.hpp>

#include "encoder.hpp"

using namespace process;

using testing::_;
using testing::Assign;
using testing::Return;
using testing::ReturnArg;


TEST(Process, thread)
{
  ThreadLocal<ProcessBase>* _process_ = new ThreadLocal<ProcessBase>();

  ProcessBase* process = new ProcessBase();

  ASSERT_TRUE(*(_process_) == NULL);

  (*_process_) = process;

  ASSERT_TRUE(*(_process_) == process);
  ASSERT_FALSE(*(_process_) == NULL);

  (*_process_) = NULL;

  ASSERT_TRUE(*(_process_) == NULL);

  delete process;
  delete _process_;
}


TEST(Process, event)
{
  Event* event = new TerminateEvent(UPID());
  EXPECT_FALSE(event->is<MessageEvent>());
  EXPECT_FALSE(event->is<ExitedEvent>());
  EXPECT_TRUE(event->is<TerminateEvent>());
  delete event;
}


TEST(Process, future)
{
  Promise<bool> promise;
  promise.set(true);
  ASSERT_TRUE(promise.future().isReady());
  EXPECT_TRUE(promise.future().get());
}


TEST(Process, associate)
{
  Promise<bool> promise1;
  Future<bool> future1(true);
  promise1.associate(future1);
  ASSERT_TRUE(promise1.future().isReady());
  EXPECT_TRUE(promise1.future().get());

  Promise<bool> promise2;
  Future<bool> future2;
  promise2.associate(future2);
  future2.discard();
  ASSERT_TRUE(promise2.future().isDiscarded());

  Promise<bool> promise3;
  Promise<bool> promise4;
  promise3.associate(promise4.future());
  promise4.fail("associate");
  ASSERT_TRUE(promise3.future().isFailed());
  EXPECT_EQ("associate", promise3.future().failure());
}


Future<std::string> itoa1(int* const& i)
{
  std::ostringstream out;
  out << *i;
  return out.str();
}


std::string itoa2(int* const& i)
{
  std::ostringstream out;
  out << *i;
  return out.str();
}


TEST(Process, then)
{
  Promise<int*> promise;

  int i = 42;

  promise.set(&i);

  Future<std::string> future = promise.future()
    .then(std::tr1::bind(&itoa1, std::tr1::placeholders::_1));

  ASSERT_TRUE(future.isReady());
  EXPECT_EQ("42", future.get());

  future = promise.future()
    .then(std::tr1::bind(&itoa2, std::tr1::placeholders::_1));

  ASSERT_TRUE(future.isReady());
  EXPECT_EQ("42", future.get());
}


class SpawnProcess : public Process<SpawnProcess>
{
public:
  MOCK_METHOD0(initialize, void(void));
  MOCK_METHOD0(finalize, void(void));
};


TEST(Process, spawn)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  SpawnProcess process;

  EXPECT_CALL(process, initialize())
    .Times(1);

  EXPECT_CALL(process, finalize())
    .Times(1);

  PID<SpawnProcess> pid = spawn(process);

  ASSERT_FALSE(!pid);

  terminate(pid);
  wait(pid);
}


class DispatchProcess : public Process<DispatchProcess>
{
public:
  MOCK_METHOD0(func0, void(void));
  MOCK_METHOD1(func1, bool(bool));
  MOCK_METHOD1(func2, Future<bool>(bool));
  MOCK_METHOD1(func3, int(int));
  MOCK_METHOD2(func4, Future<bool>(bool, int));
};


TEST(Process, dispatch)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  DispatchProcess process;

  EXPECT_CALL(process, func0())
    .Times(1);

  EXPECT_CALL(process, func1(_))
    .WillOnce(ReturnArg<0>());

  EXPECT_CALL(process, func2(_))
    .WillOnce(ReturnArg<0>());

  PID<DispatchProcess> pid = spawn(&process);

  ASSERT_FALSE(!pid);

  dispatch(pid, &DispatchProcess::func0);

  Future<bool> future;

  future = dispatch(pid, &DispatchProcess::func1, true);

  EXPECT_TRUE(future.get());
  
  future = dispatch(pid, &DispatchProcess::func2, true);

  EXPECT_TRUE(future.get());

  terminate(pid);
  wait(pid);
}


TEST(Process, defer1)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  DispatchProcess process;

  EXPECT_CALL(process, func0())
    .Times(1);

  EXPECT_CALL(process, func1(_))
    .WillOnce(ReturnArg<0>());

  EXPECT_CALL(process, func2(_))
    .WillOnce(ReturnArg<0>());

  EXPECT_CALL(process, func4(_, _))
    .WillRepeatedly(ReturnArg<0>());

  PID<DispatchProcess> pid = spawn(&process);

  ASSERT_FALSE(!pid);

  {
    deferred<void(void)> func0 =
      defer(pid, &DispatchProcess::func0);
    func0();
  }

  Future<bool> future;

  {
    deferred<Future<bool>(void)> func1 =
      defer(pid, &DispatchProcess::func1, true);
    future = func1();
    EXPECT_TRUE(future.get());
  }

  {
    deferred<Future<bool>(void)> func2 =
      defer(pid, &DispatchProcess::func2, true);
    future = func2();
    EXPECT_TRUE(future.get());
  }

  {
    deferred<Future<bool>(void)> func4 =
      defer(pid, &DispatchProcess::func4, true, 42);
    future = func4();
    EXPECT_TRUE(future.get());
  }

  {
    deferred<Future<bool>(bool)> func4 =
      defer(pid, &DispatchProcess::func4, std::tr1::placeholders::_1, 42);
    future = func4(false);
    EXPECT_FALSE(future.get());
  }

  {
    deferred<Future<bool>(int)> func4 =
      defer(pid, &DispatchProcess::func4, true, std::tr1::placeholders::_1);
    future = func4(42);
    EXPECT_TRUE(future.get());
  }

  // only take const &!

  terminate(pid);
  wait(pid);
}


template <typename T>
void set(T* t1, const T& t2)
{
  *t1 = t2;
}


class DeferProcess : public Process<DeferProcess>
{
public:
  DeferProcess(volatile bool* _bool1, volatile bool* _bool2)
    : bool1(_bool1), bool2(_bool2) {}

protected:
  virtual void initialize()
  {
    deferred<void(bool)> set1 =
      defer(std::tr1::function<void(bool)>(
                std::tr1::bind(&set<volatile bool>,
                               bool1,
                               std::tr1::placeholders::_1)));

    set1(true);

    deferred<void(bool)> set2 =
      defer(std::tr1::function<void(bool)>(
                std::tr1::bind(&set<volatile bool>,
                               bool2,
                               std::tr1::placeholders::_1)));

    set2(true);
  }

private:
  volatile bool* bool1;
  volatile bool* bool2;
};


TEST(Process, defer2)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  volatile bool bool1 = false;
  volatile bool bool2 = false;

  DeferProcess process(&bool1, &bool2);

  PID<DeferProcess> pid = spawn(process);

  while (!bool1);
  while (!bool2);

  terminate(pid);
  wait(pid);

  bool1 = false;
  bool2 = false;

  deferred<void(bool)> set1 =
    defer(std::tr1::function<void(bool)>(
              std::tr1::bind(&set<volatile bool>,
                             &bool1,
                             std::tr1::placeholders::_1)));

  set1(true);

  deferred<void(bool)> set2 =
    defer(std::tr1::function<void(bool)>(
              std::tr1::bind(&set<volatile bool>,
                             &bool2,
                             std::tr1::placeholders::_1)));

  set2(true);

  while (!bool1);
  while (!bool2);
}


class HandlersProcess : public Process<HandlersProcess>
{
public:
  HandlersProcess()
  {
    install("func", &HandlersProcess::func);
  }

  MOCK_METHOD2(func, void(const UPID&, const std::string&));
};


TEST(Process, handlers)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  HandlersProcess process;

  EXPECT_CALL(process, func(_, _))
    .Times(1);

  PID<HandlersProcess> pid = spawn(&process);

  ASSERT_FALSE(!pid);

  post(pid, "func");

  terminate(pid, false);
  wait(pid);
}


class BaseProcess : public Process<BaseProcess>
{
public:
  virtual void func() = 0;
  MOCK_METHOD0(foo, void());
};


class DerivedProcess : public BaseProcess
{
public:
  DerivedProcess() {}
  MOCK_METHOD0(func, void());
};


TEST(Process, inheritance)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  DerivedProcess process;

  EXPECT_CALL(process, func())
    .Times(2);

  EXPECT_CALL(process, foo())
    .Times(1);

  PID<DerivedProcess> pid1 = spawn(&process);

  ASSERT_FALSE(!pid1);

  dispatch(pid1, &DerivedProcess::func);

  PID<BaseProcess> pid2(process);
  PID<BaseProcess> pid3 = pid1;

  ASSERT_EQ(pid2, pid3);

  dispatch(pid3, &BaseProcess::func);
  dispatch(pid3, &BaseProcess::foo);

  terminate(pid1, false);
  wait(pid1);
}


TEST(Process, thunk)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  struct Thunk
  {
    static int run(int i)
    {
      return i;
    }

    static int run(int i, int j)
    {
      return run(i + j);
    }
  };

  int result = run(&Thunk::run, 21, 21).get();

  EXPECT_EQ(42, result);
}


class DelegatorProcess : public Process<DelegatorProcess>
{
public:
  DelegatorProcess(const UPID& delegatee)
  {
    delegate("func", delegatee);
  }
};


class DelegateeProcess : public Process<DelegateeProcess>
{
public:
  DelegateeProcess()
  {
    install("func", &DelegateeProcess::func);
  }

  MOCK_METHOD2(func, void(const UPID&, const std::string&));
};


TEST(Process, delegate)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  DelegateeProcess delegatee;
  DelegatorProcess delegator(delegatee.self());

  EXPECT_CALL(delegatee, func(_, _))
    .Times(1);

  spawn(&delegator);
  spawn(&delegatee);

  post(delegator.self(), "func");

  terminate(delegator, false);
  wait(delegator);

  terminate(delegatee, false);
  wait(delegatee);
}


// class TerminateProcess : public Process<TerminateProcess>
// {
// public:
//   TerminateProcess(Latch* _latch) : latch(_latch) {}

// protected:
//   virtual void operator () ()
//   {
//     latch->await();
//     receive();
//     EXPECT_EQ(TERMINATE, name());
//   }

// private:
//   Latch* latch;
// };


// TEST(Process, terminate)
// {
//   ASSERT_TRUE(GTEST_IS_THREADSAFE);

//   Latch latch;

//   TerminateProcess process(&latch);

//   spawn(&process);

//   post(process.self(), "one");
//   post(process.self(), "two");
//   post(process.self(), "three");

//   terminate(process.self());

//   latch.trigger();
  
//   wait(process.self());
// }


class TimeoutProcess : public Process<TimeoutProcess>
{
public:
  MOCK_METHOD0(timeout, void());
};


TEST(Process, delay)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  Clock::pause();

  volatile bool timeoutCalled = false;

  TimeoutProcess process;

  EXPECT_CALL(process, timeout())
    .WillOnce(Assign(&timeoutCalled, true));

  spawn(process);

  double seconds = 5.0;

  delay(seconds, process.self(), &TimeoutProcess::timeout);

  Clock::advance(seconds);

  while (!timeoutCalled);

  terminate(process);
  wait(process);

  Clock::resume();
}


class OrderProcess : public Process<OrderProcess>
{
public:
  void order(const PID<TimeoutProcess>& pid)
  {
    // TODO(benh): Add a test which uses 'send' instead of dispatch.
    dispatch(pid, &TimeoutProcess::timeout);
  }
};


TEST(Process, order)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  Clock::pause();

  TimeoutProcess process1;

  volatile bool timeoutCalled = false;

  EXPECT_CALL(process1, timeout())
    .WillOnce(Assign(&timeoutCalled, true));

  spawn(process1);

  double now = Clock::now(&process1);

  double seconds = 1.0;

  Clock::advance(1.0);

  EXPECT_EQ(now, Clock::now(&process1));

  OrderProcess process2;
  spawn(process2);

  dispatch(process2, &OrderProcess::order, process1.self());

  while (!timeoutCalled);

  EXPECT_EQ(now + seconds, Clock::now(&process1));

  terminate(process1);
  wait(process1);

  terminate(process2);
  wait(process2);

  Clock::resume();
}


class DonateProcess : public Process<DonateProcess>
{
public:
  void donate()
  {
    DonateProcess process;
    spawn(process);
    terminate(process);
    wait(process);
  }
};


TEST(Process, donate)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  DonateProcess process;
  spawn(process);

  dispatch(process, &DonateProcess::donate);

  terminate(process, false);
  wait(process);
}


class ExitedProcess : public Process<ExitedProcess>
{
public:
  ExitedProcess(const UPID& pid) { link(pid); }

  MOCK_METHOD1(exited, void(const UPID&));
};


TEST(Process, exited)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  UPID pid = spawn(new ProcessBase(), true);

  ExitedProcess process(pid);

  volatile bool exitedCalled = false;

  EXPECT_CALL(process, exited(pid))
    .WillOnce(Assign(&exitedCalled, true));

  spawn(process);

  terminate(pid);

  while (!exitedCalled);

  terminate(process);
  wait(process);
}


TEST(Process, select)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  Promise<int> promise1;
  Promise<int> promise2;
  Promise<int> promise3;
  Promise<int> promise4;

  std::set<Future<int> > futures;
  futures.insert(promise1.future());
  futures.insert(promise2.future());
  futures.insert(promise3.future());
  futures.insert(promise4.future());

  promise1.set(42);

  Future<Future<int> > future = select(futures);

  EXPECT_TRUE(future.await());
  EXPECT_TRUE(future.isReady());
  EXPECT_TRUE(future.get().isReady());
  EXPECT_EQ(42, future.get().get());
}


TEST(Process, collect)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  Promise<int> promise1;
  Promise<int> promise2;
  Promise<int> promise3;
  Promise<int> promise4;

  std::list<Future<int> > futures;
  futures.push_back(promise1.future());
  futures.push_back(promise2.future());
  futures.push_back(promise3.future());
  futures.push_back(promise4.future());

  promise1.set(1);
  promise2.set(2);
  promise3.set(3);
  promise4.set(4);

  Future<std::list<int> > future = collect(futures);

  EXPECT_TRUE(future.await());
  EXPECT_TRUE(future.isReady());

  std::list<int> values;
  values.push_back(1);
  values.push_back(2);
  values.push_back(3);
  values.push_back(4);

  EXPECT_EQ(values, future.get());
}


class SettleProcess : public Process<SettleProcess>
{
public:
  SettleProcess() : calledDispatch(false) {}

  virtual void initialize()
  {
    usleep(10000);
    delay(0.0, self(), &SettleProcess::afterDelay);
  }

  void afterDelay()
  {
    dispatch(self(), &SettleProcess::afterDispatch);
    usleep(10000);
    TimeoutProcess timeoutProcess;
    spawn(timeoutProcess);
    terminate(timeoutProcess);
    wait(timeoutProcess);
  }

  void afterDispatch()
  {
    usleep(10000);
    calledDispatch = true;
  }

  volatile bool calledDispatch;
};


TEST(Process, settle)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  // Try 100 times to hit a race.
  for (int i = 0; i < 100; ++i) {
    Clock::pause();
    SettleProcess process;
    spawn(process);
    Clock::settle();
    ASSERT_TRUE(process.calledDispatch);
    terminate(process);
    wait(process);
    Clock::resume();
  }
}


// #define ENUMERATE1(item) item##1
// #define ENUMERATE2(item) ENUMERATE1(item), item##2
// #define ENUMERATE3(item) ENUMERATE2(item), item##3
// #define ENUMERATE4(item) ENUMERATE3(item), item##4
// #define ENUMERATE5(item) ENUMERATE4(item), item##5
// #define ENUMERATE6(item) ENUMERATE5(item), item##6
// #define ENUMERATE(item, n) ENUMERATE##n(item)

// #define GenerateVoidDispatch(n)                                         \
//   template <typename T,                                                 \
//             ENUM(typename P, n),                                        \
//             ENUM(typename A, n)>                                        \
//   void dispatch(const PID<T>& pid,                                      \
//                 void (T::*method)(ENUM(P, n)),                          \
//                 ENUM(A, a, n))                                          \
//   {                                                                     \
//     std::tr1::function<void(T*)> thunk =                                \
//       std::tr1::bind(method, std::tr1::placeholders::_1, ENUM(a, 5));   \
//                                                                         \
//     std::tr1::function<void(ProcessBase*)>* dispatcher =                \
//       new std::tr1::function<void(ProcessBase*)>(                       \
//           std::tr1::bind(&internal::vdispatcher<T>,                     \
//                          std::tr1::placeholders::_1,                    \
//                          thunk));                                       \
//                                                                         \
//     internal::dispatch(pid, dispatcher);                                \
// }

// }


TEST(Process, pid)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  TimeoutProcess process;

  PID<TimeoutProcess> pid = process;

//   foo(process, &TimeoutProcess::timeout);
  //  dispatch(process, &TimeoutProcess::timeout);
}


class Listener1 : public Process<Listener1>
{
public:
  virtual void event1() = 0;
};


class Listener2 : public Process<Listener2>
{
public:
  virtual void event2() = 0;
};


class MultipleListenerProcess
  : public Process<MultipleListenerProcess>,
    public Listener1,
    public Listener2
{
public:
  MOCK_METHOD0(event1, void());
  MOCK_METHOD0(event2, void());
};


TEST(Process, listener)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  MultipleListenerProcess process;

  EXPECT_CALL(process, event1())
    .Times(1);

  EXPECT_CALL(process, event2())
    .Times(1);

  spawn(process);

  dispatch(PID<Listener1>(process), &Listener1::event1);
  dispatch(PID<Listener2>(process), &Listener2::event2);
  
  terminate(process, false);
  wait(process);
}


class EventReceiver
{
public:
  MOCK_METHOD1(event1, void(int));
  MOCK_METHOD1(event2, void(const std::string&));
};


TEST(Process, executor)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  volatile bool event1Called = false;
  volatile bool event2Called = false;

  EventReceiver receiver;

  EXPECT_CALL(receiver, event1(42))
    .WillOnce(Assign(&event1Called, true));

  EXPECT_CALL(receiver, event2("event2"))
    .WillOnce(Assign(&event2Called, true));

  Executor executor;

  deferred<void(int)> event1 =
    executor.defer(std::tr1::bind(&EventReceiver::event1,
                                  &receiver,
                                  std::tr1::placeholders::_1));

  event1(42);

  deferred<void(const std::string&)> event2 =
    executor.defer(std::tr1::bind(&EventReceiver::event2,
                                  &receiver,
                                  std::tr1::placeholders::_1));

  event2("event2");

  while (!event1Called);
  while (!event2Called);
}


class RemoteProcess : public Process<RemoteProcess>
{
public:
  RemoteProcess()
  {
    install("handler", &RemoteProcess::handler);
  }

  MOCK_METHOD2(handler, void(const UPID&, const std::string&));
};


TEST(Process, remote)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  RemoteProcess process;

  volatile bool handlerCalled = false;

  EXPECT_CALL(process, handler(_, _))
    .WillOnce(Assign(&handlerCalled, true));

  spawn(process);

  int s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

  ASSERT_LE(0, s);

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = PF_INET;
  addr.sin_port = htons(process.self().port);
  addr.sin_addr.s_addr = process.self().ip;

  ASSERT_EQ(0, connect(s, (sockaddr*) &addr, sizeof(addr)));

  Message message;
  message.name = "handler";
  message.from = UPID();
  message.to = process.self();

  const std::string& data = MessageEncoder::encode(&message);

  ASSERT_EQ(data.size(), write(s, data.data(), data.size()));

  ASSERT_EQ(0, close(s));

  while (!handlerCalled);

  terminate(process);
  wait(process);
}


class HttpProcess : public Process<HttpProcess>
{
public:
  HttpProcess()
  {
    route("/handler", &HttpProcess::handler);
  }

  MOCK_METHOD1(handler, Future<http::Response>(const http::Request&));
};


TEST(Process, http)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  HttpProcess process;

  EXPECT_CALL(process, handler(_))
    .WillOnce(Return(http::OK()));

  spawn(process);

  int s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

  ASSERT_LE(0, s);

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = PF_INET;
  addr.sin_port = htons(process.self().port);
  addr.sin_addr.s_addr = process.self().ip;

  ASSERT_EQ(0, connect(s, (sockaddr*) &addr, sizeof(addr)));

  std::ostringstream out;

  out << "GET /" << process.self().id << "/" << "handler"
      << " HTTP/1.0\r\n"
      << "Connection: Keep-Alive\r\n"
      << "\r\n";

  const std::string& data = out.str();

  ASSERT_EQ(data.size(), write(s, data.data(), data.size()));

  std::string response = "HTTP/1.1 200 OK";

  char temp[response.size()];

  ASSERT_LT(0, read(s, temp, response.size()));

  ASSERT_EQ(response, std::string(temp, response.size()));

  ASSERT_EQ(0, close(s));

  terminate(process);
  wait(process);
}


TEST(Process, poll)
{
  ASSERT_TRUE(GTEST_IS_THREADSAFE);

  int pipes[2];
  pipe(pipes);

  Future<short> future = io::poll(pipes[0], io::READ);

  EXPECT_FALSE(future.isReady());

  ASSERT_EQ(3, write(pipes[1], "hi", 3));

  future.await();

  ASSERT_TRUE(future.isReady());
  EXPECT_EQ(io::READ, future.get());

  close(pipes[0]);
  close(pipes[1]);
}


int main(int argc, char** argv)
{
  // Initialize Google Mock/Test.
  testing::InitGoogleMock(&argc, argv);

  return RUN_ALL_TESTS();
}
