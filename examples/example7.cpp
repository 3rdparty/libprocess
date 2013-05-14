#include <iostream>

#include <process/defer.hpp>
#include <process/dispatch.hpp>
#include <process/future.hpp>
#include <process/pid.hpp>
#include <process/process.hpp>

#include <stout/nothing.hpp>

using namespace process;

using std::string;

class StoreProcess : public Process<StoreProcess>
{
public:
  void put(int i) { promise.set(i); }
  Future<int> get() { return promise.future(); }

private:
  Promise<int> promise;
};


class ReceiveProcess : public Process<ReceiveProcess>
{
public:
  ReceiveProcess(const PID<StoreProcess>& pid)
  {
    this->pid = pid;
  }

protected:
  virtual void initialize()
  {
    dispatch(pid, &StoreProcess::get)
      .then(defer([this] (int i) mutable -> Nothing {
        std::cout << "received " << i << std::endl;
        terminate(self());
        return Nothing();
      }));
  }

private:
  PID<StoreProcess> pid;
};


int main(int argc, char** argv)
{
  StoreProcess storer;
  spawn(storer);

  ReceiveProcess receiver(storer.self());
  spawn(receiver);

  dispatch(storer, &StoreProcess::put, 42);

  wait(receiver);

  terminate(storer);
  wait(storer);

  return 0;
}
