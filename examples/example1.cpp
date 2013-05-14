#include <process/process.hpp>

using namespace process;


class MyProcess : public Process<MyProcess> {};


int main(int argc, char** argv)
{
  MyProcess process;

  spawn(process);
  terminate(process);
  wait(process);

  return 0;
}
