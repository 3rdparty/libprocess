#include <iostream>

#include <process/dispatch.hpp>
#include <process/future.hpp>
#include <process/process.hpp>

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


int main(int argc, char** argv)
{
  StoreProcess process;

  spawn(process);

  dispatch(process, &StoreProcess::put, 42);

  Future<int> i = dispatch(process, &StoreProcess::get);

  i.await();

  std::cout << i.get() << std::endl;

  terminate(process);
  wait(process);

  return 0;
}
