//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphLivePrint.h"

PerfGraphLivePrint::PerfGraphLivePrint(PerfGraph & perf_graph, MooseApp & app) : ConsoleStreamInterface(app), _perf_graph(perf_graph),

                                                                 _execution_list(perf_graph._execution_list),
                                                                                 _done_future(perf_graph._done.get_future()),
                                                                 _id_to_section_info(perf_graph._id_to_section_info),
                                                                 _stack_level(0),
                                                                                 _last_execution_list_end(0),
                                                                                 _last_printed_increment(NULL)
{
}

void
PerfGraphLivePrint::printLiveMessage(PerfGraph::SectionIncrement & section_increment)
{
  // This line is different - need to finish the last line
  if (_last_printed_increment && _last_printed_increment != &section_increment && (_last_printed_increment->_state == PerfGraph::IncrementState::printed || _last_printed_increment->_state == PerfGraph::IncrementState::continued) && _id_to_section_info[_last_printed_increment->_id]._print_dots)
    _console << '\n';

  if(_last_printed_increment && _last_printed_increment == &section_increment && (section_increment._state != PerfGraph::IncrementState::finished && (section_increment._state == PerfGraph::IncrementState::printed || section_increment._state == PerfGraph::IncrementState::continued))) // Add dots
  {
    if (_id_to_section_info[section_increment._id]._print_dots)
    {
      _console << " .";
      section_increment._num_dots++;
    }
  }
  else if (section_increment._state == PerfGraph::IncrementState::printed || section_increment._state == PerfGraph::IncrementState::continued) // Printed before so print "Still"
  {
    _console << std::string(2 * section_increment._print_stack_level, ' ') << "Still " << _id_to_section_info[section_increment._id]._live_message;

    // If we're not printing dots - just finish the line
    if (!_id_to_section_info[section_increment._id]._print_dots)
      _console << '\n';

    section_increment._num_dots = 0;
  }
  else // Just print the message
  {
    _console << std::string(2 * section_increment._print_stack_level, ' ') << _id_to_section_info[section_increment._id]._live_message;

    // If we're not printind dots - just finish the line
    if (!_id_to_section_info[section_increment._id]._print_dots)
      _console << '\n';

    section_increment._num_dots = 0;
  }

  section_increment._state = PerfGraph::IncrementState::printed;

  _last_printed_increment = &section_increment;

  _printed = true;
}

void
PerfGraphLivePrint::printStats(PerfGraph::SectionIncrement & section_increment_start, PerfGraph::SectionIncrement & section_increment_finish)
{
  auto time_increment = std::chrono::duration<double>(section_increment_finish._time - section_increment_start._time).count();
  auto memory_increment = section_increment_finish._memory - section_increment_start._memory;

  auto num_horizontal_chars = _id_to_section_info[section_increment_start._id]._live_message.size() + (2 * section_increment_start._print_stack_level);

  if (!_id_to_section_info[section_increment_start._id]._print_dots || (_last_printed_increment && _last_printed_increment != &section_increment_start)) // Print Finished
  {
    _console << '\n' << std::string(2 * section_increment_start._print_stack_level, ' ') << "Finished " << _id_to_section_info[section_increment_start._id]._live_message;

    num_horizontal_chars += std::string("Finished ").size();
  }
  else
    num_horizontal_chars += 2 * section_increment_start._num_dots;

  _console << std::setw(WRAP_LENGTH - num_horizontal_chars) << ' ' << " [" << COLOR_YELLOW << std::setw(6)
           << std::fixed << std::setprecision(2) << time_increment << " s" << COLOR_DEFAULT
           << ']' << " [" << COLOR_YELLOW << std::setw(5) << std::fixed << memory_increment
           << " MB" << COLOR_DEFAULT << ']';


  // If we're not printing dots - just finish the line
  if (!_id_to_section_info[section_increment_start._id]._print_dots)
    _console << '\n';

  _last_printed_increment = &section_increment_start;

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

    // Note: this will reset the state of a "continued" section to "printed" - so that it can be continued again
    section._state = PerfGraph::IncrementState::printed;
  }
}

void
PerfGraphLivePrint::inSamePlace()
{
  // Only print if nothing else has been printed in the meantime
  if (_last_num_printed == _console.numPrinted())
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
      // Get the beginning information for this section... it is the thing currently on the top of the stack
      auto & section_increment_start = *_print_thread_stack[_stack_level - 1];

      auto time_increment = std::chrono::duration<double>(section_increment._time - section_increment_start._time).count();
      auto memory_increment = section_increment._memory - section_increment_start._memory;

      // If it has already been printed or meets our criteria then print it and finish it
      if (section_increment_start._state == PerfGraph::IncrementState::printed || section_increment_start._state == PerfGraph::IncrementState::continued || time_increment > 1. || memory_increment > 100)
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
  while(_done_future.wait_for(std::chrono::duration<Real>(0.)) == std::future_status::timeout)
  {
    std::unique_lock<std::mutex> lock(_perf_graph._print_thread_mutex);

    // Wait for one second, or until notified that a section is finished
    // For a section to have finished the execution list has to have been appended to
    // This keeps spurious wakeups from happening
    _perf_graph._finished_section.wait_for(lock, std::chrono::duration<Real>(1.), [this]{

        // The end will be one past the last
        this->_current_execution_list_end = _perf_graph._execution_list_end.load(std::memory_order_relaxed);

        return this->_last_execution_list_end != this->_current_execution_list_end;

      });

    // The last entry in the current execution list for convenience
    _current_execution_list_last = _current_execution_list_end - 1 >= 0 ? _current_execution_list_end - 1 : MAX_EXECUTION_LIST_SIZE;

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

    // Are we still sitting in the same place as the last iteration?  If so, we need to print progress and exit
    if (_last_execution_list_end == _current_execution_list_end)
      inSamePlace();

    // This means that new stuff has been added to the execution list.  We need to iterate through it, modifying the stack and printing anything that needs printing
    iterateThroughExecutionList();

    // Make sure that everything comes out on the console
    if (_printed)
    {
      _console << std::flush;
      _last_num_printed = _console.numPrinted();
    }

    _last_execution_list_end = _current_execution_list_end;
  }
}
