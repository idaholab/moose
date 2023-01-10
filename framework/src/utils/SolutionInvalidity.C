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
  if (_counts.size() <= _invalid_solution_id)
    _counts.resize(_invalid_solution_id + 1);

  ++_counts[_invalid_solution_id].counts;
}

bool
SolutionInvalidity::solutionInvalid() const
{
  for (auto & entry : _counts)
    if (entry.counts)
      return true;
  return false;
}

void
SolutionInvalidity::resetSolutionInvalid()
{
  for (auto & entry : _counts)
    entry.counts = 0;
}

void
SolutionInvalidity::resetSolutionInvalidTimeIter()
{
  for (auto & entry : _counts)
    entry.timeiter_counts = 0;
}

void
SolutionInvalidity::solutionInvalidAccumulation()
{
  for (auto & entry : _counts)
  {
    entry.timeiter_counts += entry.counts;
    entry.total_counts += entry.counts;
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
  const auto & info = _solution_invalidity_registry.item(_invalid_solution_id);
  _console << info._name << ": " << info._message << "\n" << std::flush;
}

SolutionInvalidity::FullTable
SolutionInvalidity::summaryTable() const
{
  FullTable vtable({"Object", "Current", "Timestep", "Total", "Message"}, 4);

  vtable.setColumnFormat({
      VariadicTableColumnFormat::AUTO, // Object Name
      VariadicTableColumnFormat::AUTO, // Current Iteration Warnings
      VariadicTableColumnFormat::AUTO, // Current Time Iteration Warnings
      VariadicTableColumnFormat::AUTO, // Total Iternation Warnings
      VariadicTableColumnFormat::AUTO, // Message
  });

  vtable.setColumnPrecision({
      1, // Object Name
      0, // Current Iternation Warnings
      0, // Current Time Iternation Warnings
      0, // Total Iteration Warnings
      1, // Message
  });

  for (const auto id : index_range(_counts))
  {
    const auto & entry = _counts[id];
    if (entry.counts > 0)
    {
      const auto & info = _solution_invalidity_registry.item(id);
      vtable.addRow(info._name,            // Object Name
                    entry.counts,          // Current Iteration Warnings
                    entry.timeiter_counts, // Current Time Iteration Warnings
                    entry.total_counts,    // Total Iternation Warnings
                    info._message          // Message
      );
    }
  }

  return vtable;
}
