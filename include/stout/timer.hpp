/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __STOUT_TIMER_HPP__
#define __STOUT_TIMER_HPP__

#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif // __MACH__

#include <sys/time.h>

#include "time.hpp"

class Timer
{
public:
  Timer() : running(false) { started.tv_sec = 0; started.tv_nsec = 0; }

  void start()
  {
    started = now();
    running = true;
  }

  void stop()
  {
    stopped = now();
    running = false;
  }

  nanoseconds elapsed()
  {
    if (!running) {
      return nanoseconds(diff(stopped, started));
    }

    return nanoseconds(diff(now(), started));
  }

private:
  static timespec now()
  {
    timespec ts;
#ifdef __MACH__
    // OS X does not have clock_gettime, use clock_get_time.
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts.tv_sec = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif // __MACH__
    return ts;
  }

  static uint64_t diff(const timespec& from, const timespec& to)
  {
    return ((from.tv_sec - to.tv_sec) * 1000000000LL)
      + (from.tv_nsec - to.tv_nsec);
  }

  bool running;
  timespec started, stopped;
};

#endif // __STOUT_TIMER_HPP__
