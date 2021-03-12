//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphLivePrint.h"

PerfGraphLivePrint::PerfGraphLivePrint(PerfGraph & perf_graph, MooseApp & app)
  : ConsoleStreamInterface(app),
    _perf_graph(perf_graph),
    _execution_list(perf_graph._execution_list),
    _done_future(perf_graph._done.get_future()),
    _destructing(perf_graph._destructing),
    _should_print(true),
    _id_to_section_info(perf_graph._id_to_section_info),
    _time_limit(perf_graph._live_print_time_limit),
    _mem_limit(perf_graph._live_print_mem_limit),
    _stack_level(0),
    _current_execution_list_end(0),
    _current_execution_list_last(0),
    _last_execution_list_end(0),
    _last_printed_increment(NULL),
    _last_num_printed(0),
    _printed(false)
{
}

void
PerfGraphLivePrint::printLiveMessage(PerfGraph::SectionIncrement & section_increment)
{
  // If the live_message is empty - just print the name
  auto message = !_id_to_section_info[section_increment._id]._live_message.empty()
                     ? _id_to_section_info[section_increment._id]._live_message
                     : _id_to_section_info[section_increment._id]._name;

  // This line is different - need to finish the last line
  if (_last_printed_increment && _last_printed_increment != &section_increment &&
      (_last_printed_increment->_state == PerfGraph::IncrementState::printed ||
       _last_printed_increment->_state == PerfGraph::IncrementState::continued) &&
      _id_to_section_info[_last_printed_increment->_id]._print_dots)
    _console << '\n';

  // Do we need to print dots?
  if (_last_printed_increment && _last_printed_increment == &section_increment &&
      (section_increment._state != PerfGraph::IncrementState::finished &&
       (section_increment._state == PerfGraph::IncrementState::printed ||
        section_increment._state == PerfGraph::IncrementState::continued)))
  {
    if (_id_to_section_info[section_increment._id]._print_dots)
    {
      _console << ".";
      section_increment._num_dots++;
    }
  }
  else if (section_increment._state == PerfGraph::IncrementState::printed ||
           section_increment._state ==
               PerfGraph::IncrementState::continued) // Printed before so print "Still"
  {
    _console << std::string(2 * section_increment._print_stack_level, ' ') << "Still " << message;

    // If we're not printing dots - just finish the line
    if (!_id_to_section_info[section_increment._id]._print_dots)
      _console << '\n';

    section_increment._num_dots = 0;
  }
  else // Just print the message
  {
    _console << std::string(2 * section_increment._print_stack_level, ' ') << message;

    // If we're not printing dots - just finish the line
    if (!_id_to_section_info[section_increment._id]._print_dots)
      _console << '\n';

    section_increment._num_dots = 0;
  }

  section_increment._state = PerfGraph::IncrementState::printed;

  _last_printed_increment = &section_increment;

  _printed = true;
}

void
PerfGraphLivePrint::printStats(PerfGraph::SectionIncrement & section_increment_start,
                               PerfGraph::SectionIncrement & section_increment_finish)
{
  // If the live_message is empty - just print the name
  auto message = !_id_to_section_info[section_increment_start._id]._live_message.empty()
                     ? _id_to_section_info[section_increment_start._id]._live_message
                     : _id_to_section_info[section_increment_start._id]._name;

  /*
  auto time_increment =
      std::chrono::duration<double>(section_increment_finish._time -
      section_increment_start._time)
          .count();
  */

  // auto memory_increment = section_increment_finish._memory
  // - section_increment_start._memory;

  auto num_horizontal_chars = message.size() + (2 * section_increment_start._print_stack_level);

  // Do we need to print "Finished"?
  // This happens after something else printed in-between when this increment started and finished
  if (!_id_to_section_info[section_increment_start._id]._print_dots ||
      (_last_printed_increment && _last_printed_increment != &section_increment_start))
  {
    if ((_last_printed_increment &&
         _last_printed_increment->_state == PerfGraph::IncrementState::printed &&
         _id_to_section_info[_last_printed_increment->_id]._print_dots))
      _console << '\n';

    _console << std::string(2 * section_increment_start._print_stack_level, ' ') << "Finished "
             << message;

    num_horizontal_chars += std::string("Finished ").size();
  }
  else
    num_horizontal_chars += section_increment_start._num_dots;

  // Actually do the printing
  //  _console << std::setw(WRAP_LENGTH - num_horizontal_chars);
  _console << ' ';

  /*
  _console << " [";
  _console << COLOR_YELLOW;
  _console << std::setw(6) << std::fixed
  << std::setprecision(2) << time_increment << " s";
  _console << COLOR_DEFAULT;
  _console << ']' << " [";
  _console << COLOR_YELLOW << std::setw(5) << std::fixed;
  _console << memory_increment;
  _console << " MB";
  _console << COLOR_DEFAULT;
  _console << ']';
  */

  // If we're not printing dots - just finish the line
  _console << std::endl;

  _last_printed_increment = &section_increment_finish;

  _printed = true;
}

void
PerfGraphLivePrint::printStack()
{
  _console << "\n\n-------\n";

  _console << "stack_level: " << _stack_level << std::endl;

  if (_stack_level < 1)
  {
    _console << "-------\n" << std::endl;
    return;
  }

  for (unsigned int s = 0; s < _stack_level; s++)
  {
    auto & section = *_print_thread_stack[s];

    _console << std::string(s * 2, ' ') << _id_to_section_info[section._id]._live_message << '\n';
  }

  _console << "-------\n" << std::endl;
}

void
PerfGraphLivePrint::printStackUpToLast()
{
  if (_stack_level < 1)
    return;

  // We need to print out everything on the stack before this that hasn't already been printed...
  for (unsigned int s = 0; s < _stack_level - 1; s++)
  {
    auto & section = *_print_thread_stack[s];

    if (section._state == PerfGraph::IncrementState::started) // Hasn't been printed at all
      printLiveMessage(section);

    // Note: this will reset the state of a "continued" section to "printed" - so that it can be
    // continued again
    section._state = PerfGraph::IncrementState::printed;
  }
}

void
PerfGraphLivePrint::inSamePlace()
{
  // If someone else printed since, then we need to start over
  if (_last_num_printed != _console.numPrinted())
    _last_printed_increment = nullptr;

  // Only print if there is something to print!
  if (_stack_level > 0)
  {
    printStackUpToLast();

    printLiveMessage(*_print_thread_stack[_stack_level - 1]);
  }
}

void
PerfGraphLivePrint::iterateThroughExecutionList()
{
  // Current position in the execution list
  auto p = _last_execution_list_end;

  while (p != _current_execution_list_end)
  {
    auto next_p = p + 1 < MAX_EXECUTION_LIST_SIZE ? p + 1 : 0;

    auto & section_increment = _execution_list[p];

    // New section, add to the stack
    if (section_increment._state == PerfGraph::IncrementState::started)
    {
      section_increment._print_stack_level = _stack_level;

      // Store this increment in the stack
      _print_thread_stack[_stack_level] = &section_increment;

      _stack_level++;
    }
    else // This means it's finished need to see if we need to print it
    {
      // Get the beginning information for this section... it is the thing currently on the top of
      // the stack
      auto & section_increment_start = *_print_thread_stack[_stack_level - 1];

      auto time_increment =
          std::chrono::duration<double>(section_increment._time - section_increment_start._time)
              .count();
      auto memory_increment = section_increment._memory - section_increment_start._memory;

      // If it has already been printed or meets our criteria then print it and finish it
      if (_perf_graph._live_print_all ||
          section_increment_start._state == PerfGraph::IncrementState::printed ||
          section_increment_start._state == PerfGraph::IncrementState::continued ||
          time_increment > _time_limit || memory_increment > _mem_limit)
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
  while (_done_future.wait_for(std::chrono::duration<Real>(0.)) == std::future_status::timeout)
  {
    std::unique_lock<std::mutex> lock(_perf_graph._print_thread_mutex);

    // Wait for one second, or until notified that a section is finished
    // For a section to have finished the execution list has to have been appended to
    // This keeps spurious wakeups from happening
    _perf_graph._finished_section.wait_for(lock, std::chrono::duration<Real>(1.), [this] {
      // The end will be one past the last
      this->_current_execution_list_end =
          _perf_graph._execution_list_end.load(std::memory_order_relaxed);

      // If we are destructing or there is new work to do... allow moving on
      return _destructing || this->_last_execution_list_end != this->_current_execution_list_end;
    });

    // If the PerfGraph is destructing and we don't have anything left to print - we need to quit
    // Otherwise, if there are still things to print - do it... afterwards, the loop above
    // will end because _done_future has been set in PerfGraph.
    if (_destructing && this->_last_execution_list_end == this->_current_execution_list_end)
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

    // This will synchronize with the thread_fence in addToExecutionList() so that all of the below
    // reads, will be reading synchronized memory
    std::atomic_thread_fence(std::memory_order_acquire);

    // Only happens if nothing has been added
    if (_current_execution_list_end == 0 && _last_execution_list_end == _current_execution_list_end)
      continue;

    _printed = false;

    // Iterate from the last thing printed (begin) to the last thing in the list (end)
    // If the time or memory of any section is above the threshold, print everything inbetween and
    // update begin

    // Are we still sitting in the same place as the last iteration?  If so, we need to print
    // progress and exit
    if (_last_execution_list_end == _current_execution_list_end)
      inSamePlace();

    // This means that new stuff has been added to the execution list.  We need to iterate through
    // it, modifying the stack and printing anything that needs printing
    iterateThroughExecutionList();

    // Make sure that everything comes out on the console
    if (_printed)
    {
      if (_should_print)
        _console << std::flush;
      _last_num_printed = _console.numPrinted();
    }

    _last_execution_list_end = _current_execution_list_end;
  }
}
