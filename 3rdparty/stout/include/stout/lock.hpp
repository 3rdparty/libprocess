#ifndef __STOUT_LOCK_HPP__
#define __STOUT_LOCK_HPP__

#include <pthread.h>

// RAII class for locking pthread_mutexes.
class Lock
{
public:
  Lock(pthread_mutex_t* _mutex)
    : mutex(_mutex)
  {
    pthread_mutex_lock(mutex);
  }

  ~Lock()
  {
    pthread_mutex_unlock(mutex);
  }

private:
  pthread_mutex_t* mutex;
};

#endif // __STOUT_LOCK_HPP__
