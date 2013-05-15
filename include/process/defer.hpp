#ifndef __PROCESS_DEFER_HPP__
#define __PROCESS_DEFER_HPP__

#include <tr1/functional>

#include <process/deferred.hpp>
#include <process/dispatch.hpp>
#include <process/executor.hpp>

#include <stout/preprocessor.hpp>

namespace process {

// The defer mechanism is very similar to the dispatch mechanism (see
// dispatch.hpp), however, rather than scheduling the method to get
// invoked, the defer mechanism returns a 'Deferred' object that when
// invoked does the underlying dispatch. Similar to dispatch, we
// provide the C++11 variadic template definitions first, and then use
// Boost preprocessor macros to provide the actual definitions.


// First, definitions of defer for methods returning void:
//
// template <typename T, typename ...P>
// Deferred<void(void)> void defer(const PID<T>& pid,
//                                 void (T::*method)(P...),
//                                 P... p)
// {
//   void (*dispatch)(const PID<T>&, void (T::*)(P...), P...) =
//     &process::template dispatch<T, P...>;

//   return Deferred<void(void)>(
//       std::tr1::bind(dispatch, pid, method, std::forward<P>(p)...));
// }

template <typename T>
_Defer<void(*(PID<T>, void (T::*)(void)))
       (const PID<T>&, void (T::*)(void))>
defer(const PID<T>& pid, void (T::*method)(void))
{
  void (*dispatch)(const PID<T>&, void (T::*)(void)) =
    &process::template dispatch<T>;
  return std::tr1::bind(dispatch, pid, method);
}

template <typename T>
_Defer<void(*(PID<T>, void (T::*)(void)))
       (const PID<T>&, void (T::*)(void))>
defer(const Process<T>& process, void (T::*method)(void))
{
  return defer(process.self(), method);
}

template <typename T>
_Defer<void(*(PID<T>, void (T::*)(void)))
       (const PID<T>&, void (T::*)(void))>
defer(const Process<T>* process, void (T::*method)(void))
{
  return defer(process->self(), method);
}

#define TEMPLATE(Z, N, DATA)                                            \
  template <typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<void(*(PID<T>,                                                 \
                void (T::*)(ENUM_PARAMS(N, P)),                         \
                ENUM_PARAMS(N, A)))                                     \
         (const PID<T>&,                                                \
          void (T::*)(ENUM_PARAMS(N, P)),                               \
          ENUM_PARAMS(N, P))>                                           \
  defer(const PID<T>& pid,                                              \
        void (T::*method)(ENUM_PARAMS(N, P)),                           \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    void (*dispatch)(const PID<T>&,                                     \
                     void (T::*)(ENUM_PARAMS(N, P)),                    \
                     ENUM_PARAMS(N, P)) =                               \
      &process::template dispatch<T, ENUM_PARAMS(N, P), ENUM_PARAMS(N, P)>; \
    return std::tr1::bind(dispatch, pid, method, ENUM_PARAMS(N, a));    \
  }                                                                     \
                                                                        \
  template <typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<void(*(PID<T>,                                                 \
                void (T::*)(ENUM_PARAMS(N, P)),                         \
                ENUM_PARAMS(N, A)))                                     \
         (const PID<T>&,                                                \
          void (T::*)(ENUM_PARAMS(N, P)),                               \
          ENUM_PARAMS(N, P))>                                           \
  defer(const Process<T>& process,                                      \
        void (T::*method)(ENUM_PARAMS(N, P)),                           \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    return defer(process.self(), method, ENUM_PARAMS(N, a));            \
  }                                                                     \
                                                                        \
  template <typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<void(*(PID<T>,                                                 \
                void (T::*)(ENUM_PARAMS(N, P)),                         \
                ENUM_PARAMS(N, A)))                                     \
         (const PID<T>&,                                                \
          void (T::*)(ENUM_PARAMS(N, P)),                               \
          ENUM_PARAMS(N, P))>                                           \
  defer(const Process<T>* process,                                      \
        void (T::*method)(ENUM_PARAMS(N, P)),                           \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    return defer(process->self(), method, ENUM_PARAMS(N, a));           \
  }

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE


// Next, definitions of defer for methods returning future:
//
// template <typename R, typename T, typename ...P>
// Deferred<Future<R>(void)> void defer(const PID<T>& pid,
//                                      Future<R> (T::*method)(P...),
//                                      P... p)
// {
//   Future<R> (*dispatch)(const PID<T>&, Future<R> (T::*)(P...), P...) =
//     &process::template dispatch<R, T, P...>;
//
//   return Deferred<Future<R>(void)>(
//       std::tr1::bind(dispatch, pid, method, std::forward<P>(p)...));
// }

template <typename R, typename T>
_Defer<Future<R>(*(PID<T>, Future<R> (T::*)(void)))
       (const PID<T>&, Future<R> (T::*)(void))>
defer(const PID<T>& pid, Future<R> (T::*method)(void))
{
  Future<R> (*dispatch)(const PID<T>&, Future<R> (T::*)(void)) =
    &process::template dispatch<R, T>;
  return std::tr1::bind(dispatch, pid, method);
}

template <typename R, typename T>
_Defer<Future<R>(*(PID<T>, Future<R> (T::*)(void)))(
           const PID<T>&, Future<R> (T::*)(void))>
defer(const Process<T>& process, Future<R> (T::*method)(void))
{
  return defer(process.self(), method);
}

template <typename R, typename T>
_Defer<Future<R>(*(PID<T>, Future<R> (T::*)(void)))
       (const PID<T>&, Future<R> (T::*)(void))>
defer(const Process<T>* process, Future<R> (T::*method)(void))
{
  return defer(process->self(), method);
}

#define TEMPLATE(Z, N, DATA)                                            \
  template <typename R,                                                 \
            typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<Future<R>(*(PID<T>,                                            \
                     Future<R> (T::*)(ENUM_PARAMS(N, P)),               \
                     ENUM_PARAMS(N, A)))                                \
         (const PID<T>&,                                                \
          Future<R> (T::*)(ENUM_PARAMS(N, P)),                          \
          ENUM_PARAMS(N, P))>                                           \
  defer(const PID<T>& pid,                                              \
        Future<R> (T::*method)(ENUM_PARAMS(N, P)),                      \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    Future<R> (*dispatch)(const PID<T>&,                                \
                          Future<R> (T::*)(ENUM_PARAMS(N, P)),          \
                          ENUM_PARAMS(N, P)) =                          \
      &process::template dispatch<R, T, ENUM_PARAMS(N, P), ENUM_PARAMS(N, P)>; \
    return std::tr1::bind(dispatch, pid, method, ENUM_PARAMS(N, a));    \
  }                                                                     \
                                                                        \
  template <typename R,                                                 \
            typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<Future<R>(*(PID<T>,                                            \
                     Future<R> (T::*)(ENUM_PARAMS(N, P)),               \
                     ENUM_PARAMS(N, A)))                                \
         (const PID<T>&,                                                \
          Future<R> (T::*)(ENUM_PARAMS(N, P)),                          \
          ENUM_PARAMS(N, P))>                                           \
  defer(const Process<T>& process,                                      \
        Future<R> (T::*method)(ENUM_PARAMS(N, P)),                      \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    return defer(process.self(), method, ENUM_PARAMS(N, a));            \
  }                                                                     \
                                                                        \
  template <typename R,                                                 \
            typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<Future<R>(*(PID<T>,                                            \
                     Future<R> (T::*)(ENUM_PARAMS(N, P)),               \
                     ENUM_PARAMS(N, A)))                                \
         (const PID<T>&,                                                \
          Future<R> (T::*)(ENUM_PARAMS(N, P)),                          \
          ENUM_PARAMS(N, P))>                                           \
  defer(const Process<T>* process,                                      \
        Future<R> (T::*method)(ENUM_PARAMS(N, P)),                      \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    return defer(process->self(), method, ENUM_PARAMS(N, a));           \
  }

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE


// Next, definitions of defer for methods returning a value:
//
// template <typename R, typename T, typename ...P>
// Deferred<Future<R>(void)> void defer(const PID<T>& pid,
//                                      R (T::*method)(P...),
//                                      P... p)
// {
//   Future<R> (*dispatch)(const PID<T>&, R (T::*)(P...), P...) =
//     &process::template dispatch<R, T, P...>;
//
//   return Deferred<Future<R>(void)>(
//       std::tr1::bind(dispatch, pid, method, std::forward<P>(p)...));
// }

template <typename R, typename T>
_Defer<Future<R>(*(PID<T>, R (T::*)(void)))
       (const PID<T>&, R (T::*)(void))>
defer(const PID<T>& pid, R (T::*method)(void))
{
  Future<R> (*dispatch)(const PID<T>&, R (T::*)(void)) =
    &process::template dispatch<R, T>;
  return std::tr1::bind(dispatch, pid, method);
}

template <typename R, typename T>
_Defer<Future<R>(*(PID<T>, R (T::*)(void)))
       (const PID<T>&, R (T::*)(void))>
defer(const Process<T>& process, R (T::*method)(void))
{
  return defer(process.self(), method);
}

template <typename R, typename T>
_Defer<Future<R>(*(PID<T>, R (T::*)(void)))
       (const PID<T>&, R (T::*)(void))>
defer(const Process<T>* process, R (T::*method)(void))
{
  return defer(process->self(), method);
}

#define TEMPLATE(Z, N, DATA)                                            \
  template <typename R,                                                 \
            typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<Future<R>(*(PID<T>,                                            \
                     R (T::*)(ENUM_PARAMS(N, P)),                       \
                     ENUM_PARAMS(N, A)))                                \
         (const PID<T>&,                                                \
          R (T::*)(ENUM_PARAMS(N, P)),                                  \
          ENUM_PARAMS(N, P))>                                           \
  defer(const PID<T>& pid,                                              \
        R (T::*method)(ENUM_PARAMS(N, P)),                              \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    Future<R> (*dispatch)(const PID<T>&,                                \
                          R (T::*)(ENUM_PARAMS(N, P)),                  \
                          ENUM_PARAMS(N, P)) =                          \
      &process::template dispatch<R, T, ENUM_PARAMS(N, P), ENUM_PARAMS(N, P)>; \
    return std::tr1::bind(dispatch, pid, method, ENUM_PARAMS(N, a));    \
  }                                                                     \
                                                                        \
  template <typename R,                                                 \
            typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<Future<R>(*(PID<T>,                                            \
                     R (T::*)(ENUM_PARAMS(N, P)),                       \
                     ENUM_PARAMS(N, A)))                                \
         (const PID<T>&,                                                \
          R (T::*)(ENUM_PARAMS(N, P)),                                  \
          ENUM_PARAMS(N, P))>                                           \
  defer(const Process<T>& process,                                      \
        R (T::*method)(ENUM_PARAMS(N, P)),                              \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    return defer(process.self(), method, ENUM_PARAMS(N, a));            \
  }                                                                     \
                                                                        \
  template <typename R,                                                 \
            typename T,                                                 \
            ENUM_PARAMS(N, typename P),                                 \
            ENUM_PARAMS(N, typename A)>                                 \
  _Defer<Future<R>(*(PID<T>,                                            \
                     R (T::*)(ENUM_PARAMS(N, P)),                       \
                     ENUM_PARAMS(N, A)))                                \
         (const PID<T>&,                                                \
          R (T::*)(ENUM_PARAMS(N, P)),                                  \
          ENUM_PARAMS(N, P))>                                           \
  defer(const Process<T>* process,                                      \
        R (T::*method)(ENUM_PARAMS(N, P)),                              \
        ENUM_BINARY_PARAMS(N, A, a))                                    \
  {                                                                     \
    return defer(process->self(), method, ENUM_PARAMS(N, a));           \
  }

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE



inline Deferred<void(void)> defer(
    const UPID& pid,
    const std::tr1::function<void(void)>& f)
{
  return Deferred<void(void)>([pid, f] () {
    std::tr1::shared_ptr<std::tr1::function<void(ProcessBase*)>> thunk(
        new std::tr1::function<void(ProcessBase*)>(
            [f] (ProcessBase* _) { f(); }));
    internal::dispatch(pid, thunk);
  });
}


// Due to a bug (http://gcc.gnu.org/bugzilla/show_bug.cgi?id=41933)
// with variadic templates and lambdas, we still need to do
// preprocessor expansions.
#define TEMPLATE(Z, N, DATA)                                            \
    template <ENUM_PARAMS(N, typename A)>                               \
    Deferred<void(ENUM_PARAMS(N, A))> defer(                            \
        const UPID& pid,                                                \
        const std::tr1::function<void(ENUM_PARAMS(N, A))>& f)           \
    {                                                                   \
      return Deferred<void(ENUM_PARAMS(N, A))>([=] (ENUM_BINARY_PARAMS(N, A, &a)) { \
        std::tr1::shared_ptr<std::tr1::function<void(ProcessBase*)>> thunk( \
              new std::tr1::function<void(ProcessBase*)>(               \
                  [=] (ProcessBase* _) { f(ENUM_PARAMS(N, a)); }));     \
        internal::dispatch(pid, thunk);                                 \
      });                                                               \
    }                                                                   \
                                                                        \
    template <typename R, ENUM_PARAMS(N, typename A)>                   \
    Deferred<Future<R>(ENUM_PARAMS(N, A))> defer(                       \
        const UPID& pid,                                                \
        const std::tr1::function<R(ENUM_PARAMS(N, A))>& f)              \
    {                                                                   \
      return Deferred<Future<R>(ENUM_PARAMS(N, A))>([=] (ENUM_BINARY_PARAMS(N, A, &a)) { \
        std::tr1::shared_ptr<Promise<R>> promise(new Promise<R>());     \
        std::tr1::shared_ptr<std::tr1::function<void(ProcessBase*)>> thunk( \
            new std::tr1::function<void(ProcessBase*)>(                 \
                [=] (ProcessBase* _) { promise->set(f(ENUM_PARAMS(N, a))); })); \
        internal::dispatch(pid, thunk);                                 \
        return promise->future();                                       \
      });                                                               \
    }                                                                   \
                                                                        \
    template <typename R, ENUM_PARAMS(N, typename A)>                   \
    Deferred<Future<R>(ENUM_PARAMS(N, A))> defer(                       \
        const UPID& pid,                                                \
        const std::tr1::function<Future<R>(ENUM_PARAMS(N, A))>& f)      \
    {                                                                   \
      return Deferred<Future<R>(ENUM_PARAMS(N, A))>([=] (ENUM_BINARY_PARAMS(N, A, &a)) { \
        std::tr1::shared_ptr<Promise<R> > promise(new Promise<R>());    \
        std::tr1::shared_ptr<std::tr1::function<void(ProcessBase*)>> thunk( \
            new std::tr1::function<void(ProcessBase*)>(                 \
                [=] (ProcessBase* _) { promise->associate(              \
                    f(ENUM_PARAMS(N, a))); }));                         \
        internal::dispatch(pid, thunk);                                 \
        return promise->future();                                       \
      });                                                               \
    }

  REPEAT_FROM_TO(1, 11, TEMPLATE, _) // Args A0 -> A9.
#undef TEMPLATE


namespace internal {
namespace types {

template <typename F, typename ...A>
std::tr1::function<void(A...)> function(void(F::*)(A...));

template <typename F, typename ...A>
std::tr1::function<void(A...)> function(void(F::*)(A...)const);

template <typename F, typename R, typename ...A>
std::tr1::function<R(A...)> function(R(F::*)(A...));

template <typename F, typename R, typename ...A>
std::tr1::function<R(A...)> function(R(F::*)(A...)const);

template <typename F, typename R, typename ...A>
std::tr1::function<Future<R>(A...)> function(Future<R>(F::*)(A...));

template <typename F, typename R, typename ...A>
std::tr1::function<Future<R>(A...)> function(Future<R>(F::*)(A...)const);

template <typename F, typename ...A>
Deferred<void(A...)> deferred(void(F::*)(A...));

template <typename F, typename ...A>
Deferred<void(A...)> deferred(void(F::*)(A...)const);

template <typename F, typename R, typename ...A>
Deferred<Future<R>(A...)> deferred(R(F::*)(A...));

template <typename F, typename R, typename ...A>
Deferred<Future<R>(A...)> deferred(R(F::*)(A...)const);

template <typename F, typename R, typename ...A>
Deferred<Future<R>(A...)> deferred(Future<R>(F::*)(A...));

template <typename F, typename R, typename ...A>
Deferred<Future<R>(A...)> deferred(Future<R>(F::*)(A...)const);

} // namespace types {
} // namespace internal {


template <typename F>
auto defer(const UPID& pid, F f)
    -> decltype(internal::types::deferred(&F::operator ()))
{
  typedef decltype(internal::types::function(&F::operator ())) Function;
  return defer(pid, Function(f));
}

} // namespace process {

#endif // __PROCESS_DEFER_HPP__
