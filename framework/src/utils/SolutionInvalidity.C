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
#include <timpi/parallel_sync.h>

// libMesh Includes
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"

SolutionInvalidity::SolutionInvalidity(MooseApp & app)
  : ConsoleStreamInterface(app),
    ParallelObject(app.comm()),
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
  libmesh_parallel_only(comm());

  bool is_invalid = false;
  for (auto & entry : _counts)
  {
    if (entry.counts)
    {
      is_invalid = true;
      break;
    }
  }

  // unsigned int invalid = is_invalid;
  comm().max(is_invalid);
  return is_invalid > 0;
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
SolutionInvalidity::sync()
{
  // to do: also send the object name, and the other two values of counts
  std::map<
      processor_id_type,
      std::vector<std::tuple<std::string, std::string, unsigned int, unsigned int, unsigned int>>>
      data_to_send;
  if (processor_id() != 0)
    for (const auto id : index_range(_counts))
    {
      const auto & entry = _counts[id];
      if (entry.counts)
      {
        const auto & info = _solution_invalidity_registry.item(id);
        data_to_send[0].emplace_back(info.object_type,
                                     info.message,
                                     entry.counts,
                                     entry.timeiter_counts,
                                     entry.total_counts);
      }
    }

  // to do: act on the data
  const auto receive_data = [this](const processor_id_type libmesh_dbg_var(pid), const auto & data)
  {
    mooseAssert(pid != 0, "Should not be used except processor 0");
    for (const auto & [object_type, message, counts, timeiter_counts, total_counts] : data)
    {
      InvalidSolutionID masterId = 0;
      const moose::internal::SolutionInvalidityName name(object_type, message);
      if (_solution_invalidity_registry.keyExists(name))
        masterId = _solution_invalidity_registry.id(name);
      else
        masterId = moose::internal::getSolutionInvalidityRegistry().registerInvalidity(object_type,
                                                                                       message);

      _counts[masterId].counts += counts;
      _counts[masterId].timeiter_counts += timeiter_counts;
      _counts[masterId].total_counts += total_counts;
    }
  };

  TIMPI::push_parallel_vector_data(comm(), data_to_send, receive_data);
}

void
SolutionInvalidity::printDebug(InvalidSolutionID _invalid_solution_id) const
{
  const auto & info = _solution_invalidity_registry.item(_invalid_solution_id);
  _console << info.object_type << ": " << info.message << "\n" << std::flush;
}

SolutionInvalidity::FullTable
SolutionInvalidity::summaryTable() const
{
  FullTable vtable({"Object", "Current", "Timestep", "Total", "Message"}, 4);

  vtable.setColumnFormat({
      VariadicTableColumnFormat::AUTO, // Object Type
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
      vtable.addRow(info.object_type,      // Object Type
                    entry.counts,          // Current Iteration Warnings
                    entry.timeiter_counts, // Current Time Iteration Warnings
                    entry.total_counts,    // Total Iternation Warnings
                    info.message           // Message
      );
    }
  }

  return vtable;
}
