//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConsoleStream.h"
#include "StreamArguments.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <future>

#define CONSOLE_TIMED_PRINT(...)                                                                   \
  TimedPrint tpc(                                                                                  \
      _console, std::chrono::duration<double>(1), std::chrono::duration<double>(1), __VA_ARGS__);

/**
 * Object to print a message after enough time has passed.
 * Should help when there is a long running process.
 * It will wait to print a message, then it will start
 * print dots.
 *
 * Use it as a scope guard.  As long as it is alive it
 * will print out periodically
 */
class TimedPrint final
{
public:
  /**
   * Start the timing and printing
   *
   * @param message The message to print out
   * @param initial_wait The amount of time (in seconds) to wait before printing
   * message
   * @param dot_interval The amount of time (in seconds) to wait before printing
   * each dot
   */
  template <class StreamType, typename... Args>
  TimedPrint(StreamType & out,
             std::chrono::duration<double> initial_wait,
             std::chrono::duration<double> dot_interval,
             Args &&... args)
  {
    // This is using move assignment
    _thread = std::thread{[&out, initial_wait, dot_interval, this, args...] {
      auto done_future = this->_done.get_future();

      if (done_future.wait_for(initial_wait) == std::future_status::timeout)
      {
        streamArguments(out, args...);
        out << std::flush;
      }
      else // This means the section ended before we printed anything... so just exit
        return;

      while (done_future.wait_for(dot_interval) == std::future_status::timeout)
        out << "." << std::flush;

      // Finish the line
      out << std::endl;
    }};
  }

  /**
   * Stop the printing
   */
  ~TimedPrint()
  {
    // Tell the thread to end
    _done.set_value(true);

    // Wait for it to end
    _thread.join();
  }

protected:
  std::promise<bool> _done;
  std::thread _thread;
};
