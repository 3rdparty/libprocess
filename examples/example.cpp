#include <process/process.hpp>

using namespace process;

using std::string;

class MyProcess : public Process<MyProcess> {};


int main(int argc, char** argv)
{
  MyProcess process;

  spawn(process);
  terminate(process);
  wait(process);

  return 0;
}
