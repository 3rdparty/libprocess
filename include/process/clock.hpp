#ifndef __PROCESS_CLOCK_HPP__
#define __PROCESS_CLOCK_HPP__

#include <process/time.hpp>

#include <stout/duration.hpp>

namespace process {

// Forward declarations.
class ProcessBase;
class Time;

class Clock
{
public:
  static Time now();
  static Time now(ProcessBase* process);
  static void pause();
  static bool paused();
  static void resume();
  static void advance(const Duration& duration);
  static void advance(ProcessBase* process, const Duration& duration);
  static void update(const Time& time);
  static void update(ProcessBase* process, const Time& time);
  static void order(ProcessBase* from, ProcessBase* to);
  static void settle();
};

} // namespace process {

#endif // __PROCESS_CLOCK_HPP__
