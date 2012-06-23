#ifndef __PROCESS_SOCKET_HPP__
#define __PROCESS_SOCKET_HPP__

#include <assert.h>
#include <unistd.h> // For close.

// An abstraction around a socket (file descriptor) that provides
// reference counting such that the socket is only closed (and thus,
// has the possiblity of being reused) after there are no more
// references.

class Socket
{
public:
  Socket()
    : refs(new int(1)), s(-1) {}

  Socket(int _s)
    : refs(new int(1)), s(_s) {}

  ~Socket()
  {
    cleanup();
  }

  Socket(const Socket& that)
  {
    copy(that);
  }

  Socket& operator = (const Socket& that)
  {
    if (this != &that) {
      cleanup();
      copy(that);
    }
    return *this;
  }

  bool operator == (const Socket& that) const
  {
    return s == that.s && refs == that.refs;
  }

  operator int () const
  {
    return s;
  }

private:
  void copy(const Socket& that)
  {
    assert(that.refs > 0);
    __sync_fetch_and_add(that.refs, 1);
    refs = that.refs;
    s = that.s;
  }

  void cleanup()
  {
    assert(refs != NULL);
    if (__sync_sub_and_fetch(refs, 1) == 0) {
      delete refs;
      if (s >= 0) {
        close(s);
      }
    }
  }

  int* refs;
  int s;
};

#endif // __PROCESS_SOCKET_HPP__
