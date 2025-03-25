//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include <numeric>

// libMesh Includes
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"

SolutionInvalidity::SolutionInvalidity(MooseApp & app)
  : ConsoleStreamInterface(app),
    ParallelObject(app.comm()),
    _solution_invalidity_registry(moose::internal::getSolutionInvalidityRegistry()),
    _has_synced(true),
    _has_solution_warning(false),
    _has_solution_error(false)
{
}

void
SolutionInvalidity::flagInvalidSolutionInternal(const InvalidSolutionID _invalid_solution_id)
{
  std::lock_guard<std::mutex> lock_id(_invalid_mutex);
  if (_counts.size() <= _invalid_solution_id)
    _counts.resize(_invalid_solution_id + 1);

  ++_counts[_invalid_solution_id].current_counts;
}

bool
SolutionInvalidity::hasInvalidSolutionWarning() const
{
  mooseAssert(_has_synced, "Has not synced");
  return _has_solution_warning;
}

bool
SolutionInvalidity::hasInvalidSolutionError() const
{
  mooseAssert(_has_synced, "Has not synced");
  return _has_solution_error;
}

bool
SolutionInvalidity::hasInvalidSolution() const
{
  return hasInvalidSolutionWarning() || hasInvalidSolutionError();
}

void
SolutionInvalidity::resetSolutionInvalidCurrentIteration()
{
  // Zero current counts
  for (auto & entry : _counts)
    entry.current_counts = 0;
}

void
SolutionInvalidity::resetSolutionInvalidTimeStep()
{
  // Reset that we have synced because we're on a new iteration
  _has_synced = false;

  for (auto & entry : _counts)
    entry.current_timestep_counts = 0;
}

void
SolutionInvalidity::solutionInvalidAccumulation()
{
  for (auto & entry : _counts)
    entry.current_timestep_counts += entry.current_counts;
}

void
SolutionInvalidity::solutionInvalidAccumulationTimeStep(const unsigned int timestep_index)
{
  for (auto & entry : _counts)
    if (entry.current_timestep_counts)
    {
      if (entry.timestep_counts.empty() ||
          entry.timestep_counts.back().timestep_index != timestep_index)
        entry.timestep_counts.emplace_back(timestep_index);
      entry.timestep_counts.back().counts = entry.current_timestep_counts;
      entry.total_counts += entry.current_timestep_counts;
    }
}

void
SolutionInvalidity::computeTotalCounts()
{
  mooseAssert(_has_synced, "Has not synced");

  for (auto & entry : _counts)
  {
    entry.total_counts = 0;
    for (auto & time_counts : entry.timestep_counts)
      entry.total_counts += time_counts.counts;
  }
}

void
SolutionInvalidity::print(const ConsoleStream & console) const
{
  console << "\nSolution Invalid Warnings:\n";
  summaryTable().print(console);
}

void
SolutionInvalidity::printHistory(const ConsoleStream & console,
                                 unsigned int & timestep_interval_size) const
{
  console << "\nSolution Invalid Warnings History:\n";
  transientTable(timestep_interval_size).print(console);
}

void
SolutionInvalidity::syncIteration()
{
  std::map<processor_id_type, std::vector<std::tuple<std::string, std::string, int, unsigned int>>>
      data_to_send;

  // Reset this as we need to see if we have new counts
  _has_solution_warning = false;
  _has_solution_error = false;

  for (const auto id : index_range(_counts))
  {
    auto & entry = _counts[id];
    if (entry.current_counts)
    {
      const auto & info = _solution_invalidity_registry.item(id);
      data_to_send[0].emplace_back(
          info.object_type, info.message, info.warning, entry.current_counts);
      entry.current_counts = 0;
    }
  }

  const auto receive_data = [this](const processor_id_type libmesh_dbg_var(pid), const auto & data)
  {
    mooseAssert(processor_id() == 0, "Should only receive on processor 0");

    for (const auto & [object_type, message, warning_int, counts] : data)
    {
      mooseAssert(counts, "Should not send data without counts");

      // We transfer this as an integer (which is guaranteed by the standard to cast to a bool)
      // because TIMPI doesn't currently support transferring bools
      const bool warning = warning_int;

      InvalidSolutionID main_id = 0;
      const moose::internal::SolutionInvalidityName name(object_type, message);
      if (_solution_invalidity_registry.keyExists(name))
      {
        main_id = _solution_invalidity_registry.id(name);
        mooseAssert(_solution_invalidity_registry.item(main_id).warning == warning,
                    "Inconsistent registration of invalidity warning and error");
      }
      else
      {
        mooseAssert(pid != 0, "Should only hit on other processors");
        main_id = moose::internal::getSolutionInvalidityRegistry().registerInvalidity(
            object_type, message, warning);
      }
      if (_counts.size() <= main_id)
        _counts.resize(main_id + 1);

      _counts[main_id].current_counts += counts;

      if (warning)
        _has_solution_warning = true;
      else
        _has_solution_error = true;
    }
  };

  // Communicate the counts
  TIMPI::push_parallel_vector_data(comm(), data_to_send, receive_data);

  // Set the state across all processors
  comm().max(_has_solution_warning);
  comm().max(_has_solution_error);

  // We've now synced
  _has_synced = true;
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
  mooseAssert(_has_synced, "Has not synced");

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
      if (entry.current_counts)
      {
        const auto & info = _solution_invalidity_registry.item(id);
        vtable.addRow(info.object_type,              // Object Type
                      entry.current_counts,          // Converged Iteration Warnings
                      entry.current_timestep_counts, // Latest Time Step Warnings
                      entry.total_counts,            // Total Iteration Warnings
                      info.message                   // Message
        );
      }
    }
  }

  return vtable;
}

SolutionInvalidity::TimeTable
SolutionInvalidity::transientTable(unsigned int & step_interval) const
{
  mooseAssert(_has_synced, "Has not synced");

  TimeTable vtable({"Object", "Time", "Stepinterval Count", "Total Count"}, 4);

  vtable.setColumnFormat({
      VariadicTableColumnFormat::AUTO, // Object information
      VariadicTableColumnFormat::AUTO, // Simulation Time Step
      VariadicTableColumnFormat::AUTO, // Latest Time Step Warnings
      VariadicTableColumnFormat::AUTO, // Total Iteration Warnings
  });

  vtable.setColumnPrecision({
      1, // Object information
      1, // Simulation Time Step
      0, // Latest Time Step Warnings
      0, // Total Iteration Warnings
  });

  if (processor_id() == 0)
  {
    for (const auto id : index_range(_counts))
    {
      const auto & entry = _counts[id];
      const auto & info = _solution_invalidity_registry.item(id);
      std::vector<unsigned int> interval_counts;
      std::vector<unsigned int> total_counts;

      if (!entry.timestep_counts.empty())
      {
        for (unsigned int timestep = 0; timestep < entry.timestep_counts.back().timestep_index;
             timestep += step_interval)
        {

          auto start_it = timestep;
          auto end_it = (timestep + step_interval < entry.timestep_counts.back().timestep_index)
                            ? start_it + step_interval
                            : entry.timestep_counts.back().timestep_index;

          int interval_sum = 0;
          for (auto ts_count : entry.timestep_counts)
          {
            if (ts_count.timestep_index >= start_it && ts_count.timestep_index < end_it)
              interval_sum += ts_count.counts;
          }

          interval_counts.push_back(interval_sum);
        }
      }

      unsigned int interval_sum = 0;
      for (unsigned int interval_index : index_range(interval_counts))
      {
        std::string interval_index_str =
            std::to_string(interval_index) + "-" + std::to_string(interval_index + step_interval);

        interval_sum += interval_counts[interval_index];
        vtable.addRow(info.object_type + " : " + info.message, // Object information
                      interval_index_str,                      // Interval Index
                      interval_counts[interval_index],         // Interval Counts
                      interval_sum                             // Total Iteration Warnings

        );
      }
    }
  }
  return vtable;
}

// Define data store structure for TimestepCounts
void
dataStore(std::ostream & stream,
          SolutionInvalidity::TimestepCounts & timestep_counts,
          void * context)
{
  dataStore(stream, timestep_counts.timestep_index, context);
  dataStore(stream, timestep_counts.counts, context);
}

// Define data load structure for TimestepCounts
void
dataLoad(std::istream & stream,
         SolutionInvalidity::TimestepCounts & timestep_counts,
         void * context)
{
  dataLoad(stream, timestep_counts.timestep_index, context);
  dataLoad(stream, timestep_counts.counts, context);
}

void
dataStore(std::ostream & stream, SolutionInvalidity & solution_invalidity, void * context)
{
  solution_invalidity.syncIteration();

  if (solution_invalidity.processor_id() != 0)
    return;

  // Build data structure for store
  std::size_t size = solution_invalidity._counts.size();
  dataStore(stream, size, context);

  for (const auto id : index_range(solution_invalidity._counts))
  {
    auto & entry = solution_invalidity._counts[id];
    const auto & info = solution_invalidity._solution_invalidity_registry.item(id);
    std::string type = info.object_type;
    std::string message = info.message;
    bool warning = info.warning;
    dataStore(stream, type, context);
    dataStore(stream, message, context);
    dataStore(stream, warning, context);
    dataStore(stream, entry.current_counts, context);
    dataStore(stream, entry.current_timestep_counts, context);
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
  bool warning;
  InvalidSolutionID id;

  // loop over and load stored data
  for (size_t i = 0; i < num_counts; i++)
  {
    dataLoad(stream, object_type, context);
    dataLoad(stream, message, context);
    dataLoad(stream, warning, context);

    const moose::internal::SolutionInvalidityName name(object_type, message);
    if (solution_invalidity._solution_invalidity_registry.keyExists(name))
      id = solution_invalidity._solution_invalidity_registry.id(name);
    else
      id = moose::internal::getSolutionInvalidityRegistry().registerInvalidity(
          object_type, message, warning);

    if (solution_invalidity._counts.size() <= id)
      solution_invalidity._counts.resize(id + 1);

    auto & entry = solution_invalidity._counts[id];
    dataLoad(stream, entry.current_counts, context);
    dataLoad(stream, entry.current_timestep_counts, context);
    dataLoad(stream, entry.timestep_counts, context);
    dataLoad(stream, entry.total_counts, context);
  }
}
