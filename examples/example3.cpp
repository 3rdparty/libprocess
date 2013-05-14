#include <process/dispatch.hpp>
#include <process/future.hpp>
#include <process/process.hpp>

using namespace process;

using std::string;

class StoreProcess : public Process<StoreProcess>
{
public:
  void put(int i) { this->i = i; }
  int get() { return this->i; }

private:
  int i;
};


int main(int argc, char** argv)
{
  StoreProcess process;

  spawn(process);

  dispatch(process, &StoreProcess::put, 42);

  Future<int> i = dispatch(process, &StoreProcess::get);

  i.await(); // ANTI-PATTERN!

  terminate(process);
  wait(process);

  return 0;
}
