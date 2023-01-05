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

void
SolutionInvalidity::flagInvalidSolutionInternal(InvalidSolutionID _invalid_solution_id)
{
  std::lock_guard<std::mutex> lock_id(_invalid_mutex);

  if (_solution_invalid_counts.size() <= _invalid_solution_id)
  {
    _solution_invalid_counts.resize(_invalid_solution_id + 1);
    ++_solution_invalid_counts[_invalid_solution_id];
  }
  else
  {
    ++_solution_invalid_counts[_invalid_solution_id];
  }
  _solution_invalid_total_counts.resize(_solution_invalid_counts.size());
  _solution_invalid_timeiter_counts.resize(_solution_invalid_counts.size());
}

bool
SolutionInvalidity::solutionInvalid() const
{
  for (const auto count : _solution_invalid_counts)
    if (count)
      return true;
  return false;
}

void
SolutionInvalidity::resetSolutionInvalid()
{
  std::fill(_solution_invalid_counts.begin(), _solution_invalid_counts.end(), 0);
}

void
SolutionInvalidity::resetSolutionInvalidTimeIter()
{
  std::fill(_solution_invalid_timeiter_counts.begin(), _solution_invalid_timeiter_counts.end(), 0);
}

void
SolutionInvalidity::solutionInvalidAccumulation()
{
  for (unsigned int i = 0; i < _solution_invalid_counts.size(); i++)
  {
    _solution_invalid_total_counts[i] += _solution_invalid_counts[i];
    _solution_invalid_timeiter_counts[i] += _solution_invalid_counts[i];
  }
}

void
SolutionInvalidity::print(const ConsoleStream & console) const
{
  console << "\nSolution Invalid Warnings:\n";
  summaryTable().print(console);
}

void
SolutionInvalidity::printDebug(InvalidSolutionID _invalid_solution_id) const
{
  _console << _solution_invalidity_registry.sectionInfo(_invalid_solution_id)._name << ": "
           << _solution_invalidity_registry.sectionInfo(_invalid_solution_id)._message << "\n"
           << std::flush;
}

SolutionInvalidity::FullTable
SolutionInvalidity::summaryTable() const
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

    for (unsigned int id = 0; id < _solution_invalid_counts.size(); id++)
    {
      if (_solution_invalid_counts[id] > 0)
      {
        vtable.addRow(_solution_invalidity_registry.sectionInfo(id)._name, // Section
                      _solution_invalid_counts[id],          // Current Interation Warnings
                      _solution_invalid_timeiter_counts[id], // Current Time Interation Warnings
                      _solution_invalid_total_counts[id],    // Total Iternation Warnings
                      _solution_invalidity_registry.sectionInfo(id)._message // Message
        );
      }
    }
  }

  return vtable;
}
