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

  comm().max(is_invalid);
  return is_invalid > 0;
}

void
SolutionInvalidity::resetSolutionInvalidCurrentIteration()
{
  for (auto & entry : _counts)
    entry.counts = 0;
}

void
SolutionInvalidity::resetSolutionInvalidTimeStep()
{
  for (auto & entry : _counts)
    entry.timestep_counts = 0;
}

void
SolutionInvalidity::solutionInvalidAccumulation()
{
  for (auto & entry : _counts)
  {
    entry.total_counts += entry.counts;
    entry.timestep_counts += entry.counts;
  }
}

void
SolutionInvalidity::solutionInvalidAccumulationTimeStep()
{
  for (auto & entry : _counts)
  {
    entry.timestep_counts += entry.counts;
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
  std::map<processor_id_type,
           std::vector<std::tuple<std::string, std::string, unsigned int, unsigned int>>>
      data_to_send;

  if (processor_id() != 0)
  {
    for (const auto id : index_range(_counts))
    {
      const auto & entry = _counts[id];
      if (entry.counts)
      {
        const auto & info = _solution_invalidity_registry.item(id);
        data_to_send[0].emplace_back(
            info.object_type, info.message, entry.counts, entry.timestep_counts);
        _counts[id].total_counts = 0;
      }
    }
  }

  const auto receive_data = [this](const processor_id_type libmesh_dbg_var(pid), const auto & data)
  {
    mooseAssert(pid != 0, "Should not be used except processor 0");

    for (const auto & [object_type, message, counts, timestep_counts] : data)
    {
      InvalidSolutionID main_id = 0;
      const moose::internal::SolutionInvalidityName name(object_type, message);
      if (_solution_invalidity_registry.keyExists(name))
        main_id = _solution_invalidity_registry.id(name);
      else
        main_id = moose::internal::getSolutionInvalidityRegistry().registerInvalidity(object_type,
                                                                                      message);
      if (_counts.size() <= main_id)
        _counts.resize(main_id + 1);

      _counts[main_id].counts += counts;
      _counts[main_id].timestep_counts += timestep_counts;
      _counts[main_id].total_counts += timestep_counts;
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
  FullTable vtable({"Object", "Converged", "Timestep", "Total", "Message"}, 4);

  vtable.setColumnFormat({
      VariadicTableColumnFormat::AUTO, // Object Type
      VariadicTableColumnFormat::AUTO, // Converged Iteration Warnings
      VariadicTableColumnFormat::AUTO, // Latest Time Step Warnings
      VariadicTableColumnFormat::AUTO, // Total Simulation Warnings
      VariadicTableColumnFormat::AUTO, // Message
  });

  vtable.setColumnPrecision({
      1, // Object Name
      0, // Converged Iteration Warnings
      0, // Latest Time Step Warnings
      0, // Total Simulation Warnings
      1, // Message
  });

  if (processor_id() == 0)
  {
    for (const auto id : index_range(_counts))
    {
      const auto & entry = _counts[id];
      if (entry.counts > 0)
      {
        const auto & info = _solution_invalidity_registry.item(id);
        vtable.addRow(info.object_type,      // Object Type
                      entry.counts,          // Converged Iteration Warnings
                      entry.timestep_counts, // Latest Time Step Warnings
                      entry.total_counts,    // Total Iteration Warnings
                      info.message           // Message
        );
      }
    }
  }

  return vtable;
}

void
dataStore(std::ostream & stream, SolutionInvalidity & solution_invalidity, void * context)
{
  if (solution_invalidity.processor_id() != 0)
    return;

  // Build data structure for store
  auto size = solution_invalidity._counts.size();
  dataStore(stream, size, context);

  for (const auto id : index_range(solution_invalidity._counts))
  {
    const auto & entry = solution_invalidity._counts[id];
    const auto & info = solution_invalidity._solution_invalidity_registry.item(id);
    std::string type = info.object_type;
    std::string message = info.message;
    dataStore(stream, type, context);
    dataStore(stream, message, context);
    dataStore(stream, entry.counts, context);
    dataStore(stream, entry.timestep_counts, context);
    dataStore(stream, entry.total_counts, context);
  }
}

void
dataLoad(std::istream & stream, SolutionInvalidity & solution_invalidity, void * context)
{
  if (solution_invalidity.processor_id() != 0)
    return;

  std::size_t num_counts;
  // load data block size
  dataLoad(stream, num_counts, context);

  std::string object_type, message;
  InvalidSolutionID id;

  // loop over and load stored data
  for (size_t i = 0; i < num_counts; i++)
  {
    dataLoad(stream, object_type, context);
    dataLoad(stream, message, context);

    const moose::internal::SolutionInvalidityName name(object_type, message);
    if (solution_invalidity._solution_invalidity_registry.keyExists(name))
      id = solution_invalidity._solution_invalidity_registry.id(name);
    else
      id =
          moose::internal::getSolutionInvalidityRegistry().registerInvalidity(object_type, message);

    if (solution_invalidity._counts.size() <= id)
      solution_invalidity._counts.resize(id + 1);

    const auto & entry = solution_invalidity._counts[id];
    dataLoad(stream, entry.counts, context);
    dataLoad(stream, entry.timestep_counts, context);
    dataLoad(stream, entry.total_counts, context);
  }
}
