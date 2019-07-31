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

#include "libmesh/auto_ptr.h" // libmesh_make_unique

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <future>

#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 9) && !defined(__INTEL_COMPILER) &&  \
    !defined(__clang__)
// See note on GCC 4.8.4 workaround below
#include <tuple>
#endif

#define CONTROLLED_CONSOLE_TIMED_PRINT(initial_wait, dot_interval, ...)                            \
  std::unique_ptr<TimedPrint> tpc =                                                                \
      _communicator.rank() == 0                                                                    \
          ? libmesh_make_unique<TimedPrint>(_console,                                              \
                                            std::chrono::duration<double>(initial_wait),           \
                                            std::chrono::duration<double>(dot_interval),           \
                                            __VA_ARGS__)                                           \
          : nullptr;

#define CONSOLE_TIMED_PRINT(...) CONTROLLED_CONSOLE_TIMED_PRINT(1, 1, __VA_ARGS__)

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
             Args &&... args_in)
  {
#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 9) && !defined(__INTEL_COMPILER) &&  \
    !defined(__clang__)
    /**
     * There's a bug in GCC 4.8.4 (our current minimum compiler as of 6/18/2019) where we can't
     * capture the argument pack in the thread lambda. The workaround is to copy the argument
     * pack into a tuple. We want to avoid this for other compilers since it is a copy.
     */
    std::tuple<Args...> args(args_in...);
#define ARGS args

#else
#define ARGS args_in...

#endif

    // This is using move assignment
    _thread = std::thread{[&out, initial_wait, dot_interval, this, ARGS] {
      const unsigned int WRAP_LENGTH = 90; // Leave a few characters for the duration
      auto done_future = this->_done.get_future();
      auto start = std::chrono::steady_clock::now();

      unsigned int offset = 0;
      if (done_future.wait_for(initial_wait) == std::future_status::timeout)
      {
        streamArguments(out, ARGS);
        offset = out.tellp();

        out << ' ' << std::flush;
        ++offset;
      }
      else // This means the section ended before we printed anything... so just exit
        return;

      while (done_future.wait_for(dot_interval) == std::future_status::timeout)
      {
        if (offset >= WRAP_LENGTH)
        {
          out << '\n';
          offset = 0;
        }
        out << '.' << std::flush;
        ++offset;
      }

      // Finish the line
      if (offset)
      {
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> duration = end - start;

        out << std::setw(WRAP_LENGTH - offset) << ' ' << " [" << COLOR_YELLOW << std::setw(6)
            << std::fixed << std::setprecision(2) << duration.count() << " s" << COLOR_DEFAULT
            << ']';
      }

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
