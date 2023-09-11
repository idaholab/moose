//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphLivePrint.h"
#include "PerfGraphRegistry.h"
#include "MooseApp.h"

PerfGraphLivePrint::PerfGraphLivePrint(PerfGraph & perf_graph, MooseApp & app)
  : ConsoleStreamInterface(app),
    _app(app),
    _perf_graph(perf_graph),
    _perf_graph_registry(moose::internal::getPerfGraphRegistry()),
    _execution_list(perf_graph._execution_list),
    _currently_destructing(false),
    _should_print(true),
    _time_limit(perf_graph._live_print_time_limit),
    _mem_limit(perf_graph._live_print_mem_limit),
    _stack_level(0),
    _current_execution_list_end(0),
    _current_execution_list_last(0),
    _last_execution_list_end(0),
    _last_printed_increment(NULL),
    _last_num_printed(0),
    _console_num_printed(0),
    _printed(false),
    _stack_top_print_dots(true)
{
}

void
PerfGraphLivePrint::printLiveMessage(PerfGraph::SectionIncrement & section_increment)
{
  auto & section_info = _perf_graph_registry.sectionInfo(section_increment._id);

  // If we're not printing dots - we shouldn't be printing the message at all
  if (!section_info._print_dots || !_stack_top_print_dots)
  {
    section_increment._state = PerfGraph::IncrementState::PRINTED;
    _last_printed_increment = &section_increment;
    return;
  }

  // If the live_message is empty - just print the name
  auto message =
      !section_info._live_message.empty() ? section_info._live_message : section_info._name;

  // If this section is just started - but other stuff has printed before we got to print its
  // message, we need to print it anyway because it could be lengthy and print unexplained
  // dots to the console (until the Finished message). "Currently" conveys the message
  // that we did not just start it, but we are doing that right now
  if (_last_num_printed <= _console_num_printed &&
      section_increment._state == PerfGraph::IncrementState::STARTED &&
      section_increment._beginning_num_printed != _console_num_printed)
  {
    message = "Currently " + message;
    // The 10 is for "Currently "
    section_increment._num_dots = 10;
  }

  // This line is different - need to finish the last line
  if (_last_printed_increment && _last_printed_increment != &section_increment &&
      (_last_printed_increment->_state == PerfGraph::IncrementState::PRINTED ||
       _last_printed_increment->_state == PerfGraph::IncrementState::CONTINUED) &&
      section_info._print_dots)
    _console << '\n';

  // Do we need to print dots?
  if (_last_printed_increment && _last_printed_increment == &section_increment &&
      (section_increment._state != PerfGraph::IncrementState::FINISHED &&
       (section_increment._state == PerfGraph::IncrementState::PRINTED ||
        section_increment._state == PerfGraph::IncrementState::CONTINUED)))
  {
    if (section_info._print_dots)
    {
      _console << ".";
      section_increment._num_dots++;
    }
  }
  else if (section_increment._state == PerfGraph::IncrementState::PRINTED ||
           section_increment._state ==
               PerfGraph::IncrementState::CONTINUED) // Printed before so print "Still"
  {
    _console << std::string(2 * section_increment._print_stack_level, ' ') << "Still " << message;

    // If we're not printing dots - just finish the line
    if (!section_info._print_dots)
      _console << '\n';

    // The 6 is for "Still "
    section_increment._num_dots = 6;
  }
  else // Just print the message
  {
    _console << std::string(2 * section_increment._print_stack_level, ' ') << message;

    // If we're not printing dots - just finish the line
    if (!section_info._print_dots)
      _console << '\n';

    // Reset the dots since we're printing, except in the "Currently" case
    if (section_increment._state != PerfGraph::IncrementState::STARTED ||
        _last_num_printed > _console_num_printed ||
        section_increment._beginning_num_printed == _console_num_printed)
      section_increment._num_dots = 0;
  }

  section_increment._state = PerfGraph::IncrementState::PRINTED;

  // Get the message to the screen
  _console << std::flush;

  // Keep track of where we printed in the console
  _last_num_printed = section_increment._beginning_num_printed = _console.numPrinted();

  _last_printed_increment = &section_increment;

  _printed = true;
}

void
PerfGraphLivePrint::printStats(PerfGraph::SectionIncrement & section_increment_start,
                               PerfGraph::SectionIncrement & section_increment_finish)
{
  if (_stack_level < 1)
    return;

  mooseAssert(_perf_graph_registry.sectionExists(section_increment_start._id),
              "Not found in map: " << section_increment_start._id);

  auto & section_info_start = _perf_graph_registry.sectionInfo(section_increment_start._id);

  // If the live_message is empty - just print the name
  auto message = !section_info_start._live_message.empty() ? section_info_start._live_message
                                                           : section_info_start._name;

  auto time_increment =
      std::chrono::duration<double>(section_increment_finish._time - section_increment_start._time)
          .count();

  auto memory_total = section_increment_finish._memory;

  auto num_horizontal_chars = message.size() + (2 * section_increment_start._print_stack_level);

  // Add multiapps prefix size
  if (!_app.isUltimateMaster())
    num_horizontal_chars += _app.name().size() + 2;

  // Do we need to print "Finished"?
  // This happens after something else printed in-between when this increment started and finished
  if (!section_info_start._print_dots ||
      (_last_printed_increment && _last_printed_increment != &section_increment_start) ||
      (section_increment_start._beginning_num_printed !=
       _console_num_printed)) // This means someone _else_ printed
  {
    // If we had printed some dots - we need to finish the line
    if (_last_printed_increment &&
        _last_printed_increment->_state == PerfGraph::IncrementState::PRINTED &&
        _perf_graph_registry.sectionInfo(_last_printed_increment->_id)._print_dots)
      _console << '\n';

    _console << std::string(2 * section_increment_start._print_stack_level, ' ') << "Finished "
             << message;

    // 9 is for "Finished "
    num_horizontal_chars += 9;
  }
  else
    num_horizontal_chars += section_increment_start._num_dots;

  // Actually do the printing
  _console << std::setw(WRAP_LENGTH - num_horizontal_chars);

  _console << " [";
  _console << COLOR_YELLOW;
  _console << std::setw(6) << std::fixed << std::setprecision(2) << time_increment << " s";
  _console << COLOR_DEFAULT;
  _console << ']' << " [";
  _console << COLOR_YELLOW << std::setw(5) << std::fixed;
  _console << memory_total;
  _console << " MB";
  _console << COLOR_DEFAULT;
  _console << ']';

  // If we're not printing dots - just finish the line
  _console << std::endl;

  _last_num_printed = _console.numPrinted();

  _last_printed_increment = &section_increment_finish;

  _printed = true;
}

void
PerfGraphLivePrint::printStackUpToLast()
{
  if (_stack_level < 1)
    return;

  // We need to print out everything on the stack before this that hasn't already been printed...
  for (unsigned int s = 0; s < _stack_level - 1; s++)
  {
    auto & section = _print_thread_stack[s];

    // Hasn't been printed at all and nothing else has been printed since this started
    if (section._state == PerfGraph::IncrementState::STARTED)
      printLiveMessage(section);

    // Note: this will reset the state of a "continued" section to "printed" - so that it can be
    // continued again
    section._state = PerfGraph::IncrementState::PRINTED;
  }
}

void
PerfGraphLivePrint::inSamePlace()
{
  // If someone else printed since, then we need to start over, and set everything on the stack to
  // printed Everything is set to printed because if something printed and we're still in the same
  // place then we need to NOT print out the beginning message
  if (_last_num_printed != _console_num_printed)
  {
    _last_printed_increment = nullptr;

    for (unsigned int s = 0; s < _stack_level; s++)
      _print_thread_stack[s]._state = PerfGraph::IncrementState::PRINTED;

    return;
  }

  // Only print if there is something to print!
  if (_stack_level > 0)
  {
    _stack_top_print_dots =
        _perf_graph_registry.sectionInfo(_print_thread_stack[_stack_level - 1]._id)._print_dots;

    printStackUpToLast();

    printLiveMessage(_print_thread_stack[_stack_level - 1]);

    // Reset this each time
    _stack_top_print_dots = true;
  }
}

void
PerfGraphLivePrint::iterateThroughExecutionList()
{
  // Current position in the execution list
  auto p = _last_execution_list_end;

  while (p != _current_execution_list_end)
  {
    // The ternary here is for wrapping around
    auto next_p = p + 1 < MAX_EXECUTION_LIST_SIZE ? p + 1 : 0;

    auto & section_increment = _execution_list[p];

    // New section, add to the stack
    if (section_increment._state == PerfGraph::IncrementState::STARTED)
    {
      section_increment._print_stack_level = _stack_level;

      // Store this increment in the stack
      _print_thread_stack[_stack_level] = section_increment;

      _stack_level++;
    }
    else // This means it's finished need to see if we need to print it
    {
      mooseAssert(_stack_level, "Popping beyond the beginning of the stack!");

      // Get the beginning information for this section... it is the thing currently on the top of
      // the stack
      auto & section_increment_start = _print_thread_stack[_stack_level - 1];

      auto time_increment =
          std::chrono::duration<double>(section_increment._time - section_increment_start._time)
              .count();
      auto memory_increment = section_increment._memory - section_increment_start._memory;

      // If it has already been printed or meets our criteria then print it and finish it
      if (_perf_graph._live_print_all ||
          section_increment_start._state == PerfGraph::IncrementState::PRINTED ||
          section_increment_start._state == PerfGraph::IncrementState::CONTINUED ||
          time_increment > _time_limit.load(std::memory_order_relaxed) ||
          memory_increment > _mem_limit.load(std::memory_order_relaxed))
      {
        printStackUpToLast();

        printStats(section_increment_start, section_increment);
      }

      _stack_level--;
    }

    p = next_p;
  }
}

void
PerfGraphLivePrint::start()
{
  // Keep going until we're signaled to end
  // Note that _currently_destructing can only be set to true in this thread
  // Which means that by the time we make it back to the top of the loop, either
  // there was nothing to process or everything has been processed.
  while (!_currently_destructing)
  {
    std::unique_lock<std::mutex> lock(_perf_graph._destructing_mutex);

    // Wait for one second, or until notified that a section is finished
    // For a section to have finished the execution list has to have been appended to
    // This keeps spurious wakeups from happening
    // Note that the `lock` is only protecting _destructing since the execution list uses atomics.
    // It must be atomic in order to keep the main thread from having to lock as it
    // executes.  The only downside to this is that it is possible for this thread to wake,
    // check the condition, miss the notification, then wait.  In our case this is not detrimental,
    // as the only thing that will happen is we will wait 1 more second.  This is also very
    // unlikely.
    // One other thing: wait_for() is not guaranteed to wait for 1 second.  "Spurious" wakeups
    // can occur - but the predicate here keeps us from doing anything in that case.
    // This will either wait until 1 second has passed, the signal is sent, _or_ a spurious
    // wakeup happens to find that there is work to do.
    _perf_graph._finished_section.wait_for(
        lock,
        std::chrono::duration<Real>(_time_limit.load(std::memory_order_relaxed)),
        [this]
        {
          // Get destructing first so that the execution_list will be in sync
          this->_currently_destructing = _perf_graph._destructing;

          // The end will be one past the last
          // This "acquire" synchronizes with the "release" in the PerfGraph
          // to ensure that all of the writes to the execution list have been
          // published to this thread for the "end" we're reading
          this->_current_execution_list_end =
              _perf_graph._execution_list_end.load(std::memory_order_acquire);

          // Save off the number of things currently printed to the console
          this->_console_num_printed = _console.numPrinted();

          // If we are destructing or there is new work to do... allow moving on
          return this->_currently_destructing ||
                 this->_last_execution_list_end != this->_current_execution_list_end;
        });

    // If the PerfGraph is destructing and we don't have anything left to print - we need to quit
    // Otherwise, if there are still things to print - do it... afterwards, the loop above
    // will end because _done_future has been set in PerfGraph.
    if (this->_currently_destructing &&
        this->_last_execution_list_end == this->_current_execution_list_end)
      return;

    // We store this off for one execution of this loop so that it's consistent all for the whole
    // iteration
    _should_print = _perf_graph._live_print_active;

    if (!_should_print)
      continue;

    // The last entry in the current execution list for convenience
    _current_execution_list_last = static_cast<long int>(_current_execution_list_end) - 1 >= 0
                                       ? _current_execution_list_end - 1
                                       : MAX_EXECUTION_LIST_SIZE;

    // Only happens if nothing has been added
    if (_current_execution_list_end == 0 && _last_execution_list_end == _current_execution_list_end)
      continue;

    _printed = false;

    // Iterate from the last thing printed (begin) to the last thing in the list (end)
    // If the time or memory of any section is above the threshold, print everything in between and
    // update begin

    // Are we still sitting in the same place as the last iteration?  If so, we need to print
    // progress and exit
    if (_last_execution_list_end == _current_execution_list_end)
      inSamePlace();

    // This means that new stuff has been added to the execution list.  We need to iterate through
    // it, modifying the stack and printing anything that needs printing
    iterateThroughExecutionList();

    _last_num_printed = _console.numPrinted();

    _last_execution_list_end = _current_execution_list_end;
  }
}
