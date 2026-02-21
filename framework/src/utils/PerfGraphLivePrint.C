//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
    _time_limit(perf_graph._live_print_time_limit),
    _mem_limit(perf_graph._live_print_mem_limit),
    _stack_level(0),
    _current_execution_list_end(0),
    _last_execution_list_end(0),
    _last_printed_increment(nullptr),
    _last_printed_increment_finished(false),
    _last_num_printed(0),
    _console_num_printed(0),
    _stack_top_print_dots(true),
    _console_lock(Moose::moose_console_mutex, std::defer_lock)
{
}

void
PerfGraphLivePrint::printLiveMessage(PerfGraph::SectionIncrement & section_increment)
{
  mooseAssert(_console_lock, "Console not locked");

  const auto & section_info = _perf_graph_registry.sectionInfo(section_increment._id);

  // If we're not printing dots - we shouldn't be printing the message at all
  if (!section_info._print_dots || !_stack_top_print_dots)
  {
    section_increment._state = PerfGraph::IncrementState::PRINTED;
    _last_printed_increment = &section_increment;
    _last_printed_increment_finished = true;
    return;
  }

  // Get the message for the section, if none just use the section name
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
    constexpr std::string_view prefix = "Currently ";
    message = std::string(prefix) + message;
    section_increment._num_dots = prefix.size();
  }

  // Piece together the output
  std::ostringstream out;
  // This line is different - need to finish the last line
  if (_last_printed_increment && !_last_printed_increment_finished &&
      _last_printed_increment != &section_increment &&
      _last_printed_increment->_state == PerfGraph::IncrementState::PRINTED &&
      section_info._print_dots)
    out << "\n";
  // Faint coloring
  out << COLOR_FAINT;
  // Do we need to print dots?
  if (_last_printed_increment && _last_printed_increment == &section_increment &&
      section_increment._state == PerfGraph::IncrementState::PRINTED)
  {
    if (section_info._print_dots)
    {
      out << COLOR_FAINT << "." << COLOR_RESET;
      section_increment._num_dots++;
    }
  }
  // Printed before so print "Still"
  else if (section_increment._state == PerfGraph::IncrementState::PRINTED)
  {
    out << formatMessage(section_increment, "Still " + message);

    // If we're not printing dots - just finish the line
    if (!section_info._print_dots)
      out << "\n";

    // The 6 is for "Still "
    section_increment._num_dots = 6;
  }
  else // Just print the message
  {
    out << formatMessage(section_increment, message);

    // If we're not printing dots - just finish the line
    if (!section_info._print_dots)
      out << "\n";

    // Reset the dots since we're printing, except in the "Currently" case
    if (section_increment._state != PerfGraph::IncrementState::STARTED ||
        _last_num_printed > _console_num_printed ||
        section_increment._beginning_num_printed == _console_num_printed)
      section_increment._num_dots = 0;
  }

  // Consider this second printed
  section_increment._state = PerfGraph::IncrementState::PRINTED;

  // Output, all together and now
  outputNow(out);

  // Keep track of where we printed in the console
  _last_num_printed = section_increment._beginning_num_printed = _console.numPrinted();

  _last_printed_increment = &section_increment;
  _last_printed_increment_finished = false;
}

void
PerfGraphLivePrint::printStats(const PerfGraph::SectionIncrement & section_increment_start,
                               const PerfGraph::SectionIncrement & section_increment_finish)
{
  mooseAssert(_console_lock, "Console not locked");

  if (_stack_level < 1)
    return;

  mooseAssert(_perf_graph_registry.sectionExists(section_increment_start._id),
              "Not found in map: " << section_increment_start._id);

  const auto & section_info_start = _perf_graph_registry.sectionInfo(section_increment_start._id);

  // If the live_message is empty - just print the name
  const auto message = !section_info_start._live_message.empty() ? section_info_start._live_message
                                                                 : section_info_start._name;
  const auto time_increment =
      std::chrono::duration<double>(section_increment_finish._time - section_increment_start._time)
          .count();

  const auto memory_total = section_increment_finish._memory;

  auto num_horizontal_chars = message.size() + section_increment_start._print_stack_level;

  // Add multiapps prefix size
  if (!_app.isUltimateMaster())
    num_horizontal_chars += _app.name().size() + 2;

  // Form output
  std::ostringstream out;
  // Do we need to print "Finished"? This happens after something else printed
  // in-between when this increment started and finished
  if (!section_info_start._print_dots ||
      (_last_printed_increment && _last_printed_increment != &section_increment_start) ||
      (section_increment_start._beginning_num_printed !=
       _console_num_printed)) // This means someone _else_ printed
  {
    // If we had printed some dots - we need to finish the line
    if (_last_printed_increment &&
        _last_printed_increment->_state == PerfGraph::IncrementState::PRINTED &&
        _last_printed_increment->_id != 0 &&
        _perf_graph_registry.sectionInfo(_last_printed_increment->_id)._print_dots)
      out << "\n";

    constexpr std::string_view prefix = "Finished ";
    out << formatMessage(section_increment_start, std::string(prefix) + message);
    num_horizontal_chars += prefix.size();
  }
  else
    num_horizontal_chars += section_increment_start._num_dots;
  // Force character wrapping
  out << std::setw(WRAP_LENGTH - num_horizontal_chars);
  // Faint coloring
  out << COLOR_FAINT;
  // Time
  out << "[" << COLOR_YELLOW << std::setw(6) << std::fixed << std::setprecision(2) << time_increment
      << " s" << COLOR_DEFAULT << ']';
  // Memory
  out << " [" << COLOR_YELLOW << std::setw(5) << std::fixed << memory_total << " MB"
      << COLOR_DEFAULT << ']';
  // Remove faint coloring
  out << COLOR_RESET;
  // New line
  out << "\n";

  // Output, all together and now
  outputNow(out);

  // Keep track of the last state when we printed so we know later on if
  // someone else printed after us
  _last_num_printed = _console.numPrinted();

  _last_printed_increment = &section_increment_finish;
  _last_printed_increment_finished = true;
}

void
PerfGraphLivePrint::printStackUpToLast()
{
  mooseAssert(_console_lock, "Console not locked");

  if (_stack_level < 1)
    return;

  // Print out everything on the stack before this that hasn't already been printed
  for (unsigned int s = 0; s < _stack_level - 1; s++)
  {
    auto & section = _print_thread_stack[s];

    // Hasn't been printed at all and nothing else has been printed since this started
    if (section._state == PerfGraph::IncrementState::STARTED)
      printLiveMessage(section);

    section._state = PerfGraph::IncrementState::PRINTED;
  }
}

void
PerfGraphLivePrint::inSamePlace()
{
  mooseAssert(_console_lock, "Console not locked");

  // If someone else printed since, then we need to start over, and set everything on the stack to
  // printed Everything is set to printed because if something printed and we're still in the same
  // place then we need to NOT print out the beginning message
  if (_last_num_printed != _console_num_printed)
  {
    _last_printed_increment = nullptr;
    _last_printed_increment_finished = false;

    for (const auto s : make_range(_stack_level))
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
  mooseAssert(_console_lock, "Console not locked");

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

      // Get the beginning information for this section... it is the thing
      // currently on the top of the stack
      const auto & section_increment_start = _print_thread_stack[_stack_level - 1];

      const bool should_print =
          // Printing everything
          _perf_graph._live_print_all ||
          // Just printed this
          section_increment_start._state == PerfGraph::IncrementState::PRINTED ||
          // Over time limit
          std::chrono::duration<double>(section_increment._time - section_increment_start._time)
                  .count() > _time_limit.load(std::memory_order_relaxed) ||
          // Over memory limit
          (section_increment._memory - section_increment_start._memory) >
              _mem_limit.load(std::memory_order_relaxed);

      // If it has already been printed or meets our criteria then print it and finish it
      if (should_print)
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
PerfGraphLivePrint::outputNow(const std::ostringstream & out)
{
  mooseAssert(_console_lock, "Should have the console lock");
  mooseAssert(_out.str().empty(), "Should be empty");

  const auto out_str = out.str();
  mooseAssert(out_str.size(), "Should not output empty message");

  _out << out_str;
  _console_lock = _console.outputGuarded(_out, std::move(_console_lock));

  mooseAssert(_out.str().empty(), "Should have output");
}

std::string
PerfGraphLivePrint::formatMessage(const PerfGraph::SectionIncrement & section_increment,
                                  const std::string & message)
{
  return COLOR_FAINT + std::string(section_increment._print_stack_level, ' ') + message +
         COLOR_RESET;
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
    std::unique_lock destructing_lock(_perf_graph._destructing_mutex);

    // Wait for five seconds (by default), or until notified that a section is finished
    // For a section to have finished the execution list has to have been appended to
    // This keeps spurious wakeups from happening
    // Note that the `lock` is only protecting _destructing since the execution list uses atomics.
    // It must be atomic in order to keep the main thread from having to lock as it
    // executes.  The only downside to this is that it is possible for this thread to wake,
    // check the condition, miss the notification, then wait.  In our case this is not detrimental,
    // as the only thing that will happen is we will wait 5 more seconds.  This is also very
    // unlikely.
    // One other thing: wait_for() is not guaranteed to wait for 5 seconds.  "Spurious" wakeups
    // can occur - but the predicate here keeps us from doing anything in that case.
    // This will either wait until 5 seconds have passed, the signal is sent, _or_ a spurious
    // wakeup happens to find that there is work to do.
    _perf_graph._finished_section.wait_for(
        destructing_lock,
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

          // If we are destructing or there is new work to do... allow moving on
          return this->_currently_destructing ||
                 this->_last_execution_list_end != this->_current_execution_list_end;
        });

    // If the PerfGraph is destructing and we don't have anything left to print - we need to quit
    // Otherwise, if there are still things to print - do it... afterwards, the loop above
    // will end because _done_future has been set in PerfGraph.
    if (_currently_destructing && _last_execution_list_end == _current_execution_list_end)
      return;

    // Only happens if nothing has been added
    if (_current_execution_list_end == 0 && _last_execution_list_end == _current_execution_list_end)
      continue;

    // Lock the console so that nothing can write while we're writing
    _console_lock.lock();
    // Keep track of the print counter at the beginning
    _console_num_printed = _console.numPrinted();

    // Are we still sitting in the same place as the last iteration? If so, we need to print
    // progress and exit
    if (_last_execution_list_end == _current_execution_list_end)
      inSamePlace();

    // New stuff has been added to the execution list. Iterate through
    // it, modifying the stack and printing anything that needs printing
    iterateThroughExecutionList();

    // Store ending state
    _last_num_printed = _console.numPrinted();
    _last_execution_list_end = _current_execution_list_end;

    // Release access to the console
    _console_lock.unlock();
  }
}
