//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionInvalidity.h"

// MOOSE Includes
#include "MooseError.h"
#include "MooseApp.h"
#include "VariadicTable.h"

// System Includes
#include <chrono>
#include <memory>

SolutionInvalidity::SolutionInvalidity(MooseApp & app)
  : ConsoleStreamInterface(app),
    _solution_invalidity_registry(moose::internal::getSolutionInvalidityRegistry())
{
}

SolutionInvalidity::~SolutionInvalidity() {}
/// Count solution invalid occurrences for each solution id
void
SolutionInvalidity::setSolutionInvalid(SolutionID _solution_id)
{
  if (_solution_invalid_counts.size() <= _solution_id)
  {
    _solution_invalid_counts.resize(_solution_id + 1);
    ++_solution_invalid_counts[_solution_id];
  }
  else
  {
    ++_solution_invalid_counts[_solution_id];
  }
  _solution_invalid_total_counts.resize(_solution_invalid_counts.size());
  _solution_invalid_timeiter_counts.resize(_solution_invalid_counts.size());
}

/// Loop over all the tracked objects and determine whether solution invalid is detected
bool
SolutionInvalidity::solutionInvalid() const
{
  unsigned int sum_of_elems = 0;

  std::for_each(_solution_invalid_counts.begin(),
                _solution_invalid_counts.end(),
                [&](int n) { sum_of_elems += n; });
  if (sum_of_elems < 1)
  {
    return false;
  }
  else
  {
    return true;
  }
}

/// Reset the number of solution invalid occurrences back to zero
void
SolutionInvalidity::resetSolutionInvalid()
{
  std::fill(_solution_invalid_counts.begin(), _solution_invalid_counts.end(), 0);
}

/// Reset the number of solution invalid occurrences back to zero
void
SolutionInvalidity::resetSolutionInvalidTimeIter()
{
  std::fill(_solution_invalid_timeiter_counts.begin(), _solution_invalid_timeiter_counts.end(), 0);
}

/// Pass the number of solution invalid occurrences from current iteration to comulative counters
void
SolutionInvalidity::solutionInvalidAccumulation()
{
  for (unsigned int i = 0; i < _solution_invalid_counts.size(); i++)
  {
    _solution_invalid_total_counts[i] += _solution_invalid_counts[i];
    _solution_invalid_timeiter_counts[i] += _solution_invalid_counts[i];
  }
}

/// Print the summary table of Solution Invalid warnings
void
SolutionInvalidity::print(const ConsoleStream & console)
{
  console << "\nSolution Invalid Warnings:\n";
  summaryTable().print(console);
}

/// Immediately print the section and message for debug purpose
void
SolutionInvalidity::printDebug(const ConsoleStream & _console, SolutionID _solution_id)
{
  // std::stringstream output;
  _console << _solution_invalidity_registry.sectionInfo(_solution_id)._name << ": "
           << _solution_invalidity_registry.sectionInfo(_solution_id)._message << "\n";
  _console << std::flush;
}

/// Store all solution invalid warning for output
SolutionInvalidity::FullTable
SolutionInvalidity::summaryTable()
{
  FullTable vtable({"Section", "Current", "Timestep", "Total", "Message"}, 4);

  vtable.setColumnFormat({
      VariadicTableColumnFormat::AUTO, // Section Name
      VariadicTableColumnFormat::AUTO, // Current Interation Warnings
      VariadicTableColumnFormat::AUTO, // Current Time Interation Warnings
      VariadicTableColumnFormat::AUTO, // Total Iternation Warnings
      VariadicTableColumnFormat::AUTO, // Message
  });

  vtable.setColumnPrecision({
      1, // Section Name
      0, // Current Iternation Warnings
      0, // Current Time Iternation Warnings
      0, // Total Interation Warnings
      1, // Message
  });

  if (_solution_invalid_counts.size() > 0)
  {

    // Now print out the sections that contain solution invalid info and occurences
    for (unsigned int id = 0; id < _solution_invalid_counts.size(); id++)
    {

      vtable.addRow(_solution_invalidity_registry.sectionInfo(id)._name, // Section
                    _solution_invalid_counts[id],          // Current Interation Warnings
                    _solution_invalid_timeiter_counts[id], // Current Time Interation Warnings
                    _solution_invalid_total_counts[id],    // Total Iternation Warnings
                    _solution_invalidity_registry.sectionInfo(id)._message // Message
      );
    }
  }

  return vtable;
}
