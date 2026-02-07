//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE Includes
#include "MooseTypes.h"
#include "MooseError.h"
#include "AnyPointer.h"
#include "SolutionInvalidityRegistry.h"
#include "ConsoleStream.h"
#include "ConsoleStreamInterface.h"

// System Includes
#include <array>
#include <atomic>
#include <thread>
#include <future>
#include <mutex>

// libMesh Includes
#include "libmesh/parallel_object.h"

// Forward Declarations
template <class... Ts>
class VariadicTable;

/**
 * The SolutionInvalidity will contain all the information about the occurrence(s) of solution
 * invalidity
 */
class SolutionInvalidity : protected ConsoleStreamInterface, public libMesh::ParallelObject
{
public:
  using SolutionInvalidityRegistry = moose::internal::SolutionInvalidityRegistry;

  /**
   * Create a new SolutionInvalidity
   */
  SolutionInvalidity(MooseApp & app);

  /// Increments solution invalid occurrences for each solution id
  void flagInvalidSolutionInternal(const InvalidSolutionID _invalid_solution_id);

  /**
   * Whether or not an invalid solution was encountered that was a warning.
   *
   * This must be called after a sync.
   */
  bool hasInvalidSolutionWarning() const;

  /**
   * Whether or not an invalid solution was encountered that was an error.
   *
   * This must be called after a sync.
   */
  bool hasInvalidSolutionError() const;

  /**
   * Whether or not any invalid solution was encountered (error or warning).
   *
   * This must be called after a sync.
   */
  bool hasInvalidSolution() const;

  /**
   * Whether or not any warning or invalid solution has ever been encountered during the simulation
   *
   * This must be called after a sync.
   */
  bool hasEverHadSolutionIssue() const;

  /// Reset the number of solution invalid occurrences back to zero for the current time step
  void resetTimeStepOccurences();

  /// Reset the number of solution invalid occurrences back to zero
  void resetIterationOccurences();

  /// Pass the number of solution invalid occurrences from current iteration to cumulative counters
  void accumulateIterationIntoTimeStepOccurences();

  /// Pass the number of solution invalid occurrences from current timestep to cumulative timestep counter (e.g. the total)
  void accumulateTimeStepIntoTotalOccurences(const unsigned int timestep_index);

  /// Struct used in InvalidCounts for storing the time history of invalid occurrences
  struct TimestepCounts
  {
    TimestepCounts() : timestep_index(std::numeric_limits<unsigned int>::max()) {}
    TimestepCounts(unsigned int timestep_index) : timestep_index(timestep_index) {}
    unsigned int timestep_index;
    unsigned int counts = 0;
  };

  /// Struct used in _counts for storing warning and invalid-solution occurrences
  struct InvalidCounts
  {
    /// Counts for the current iteration (depends on the count, but usually linear iteration)
    unsigned int current_counts = 0;
    /// Counts for the current time step
    unsigned int current_timestep_counts = 0;
    /// Total counts across the entire simulation
    unsigned int total_counts = 0;
    /// Keep track of the occurences across all time steps
    std::vector<TimestepCounts> timestep_counts;
  };

  /// Access the private solution invalidity counts
  const std::vector<InvalidCounts> & counts() const { return _counts; }

  /**
   * Print the summary table of Solution Invalid warnings
   * @param console The output stream to output to
   */
  void print(const ConsoleStream & console) const;

  /**
   * Print the time history table of Solution Invalid warnings
   * @param console The output stream to output to
   */
  void printHistory(const ConsoleStream & console, unsigned int & timestep_interval_size) const;

  /**
   * Immediately print the section and message for debug purpose
   */
  void printDebug(InvalidSolutionID _invalid_solution_id) const;

  /**
   * Sync iteration counts to main processor
   * Sum across all processors
   */
  void syncIteration();

  /// Whether the solution invalidity has synchronized iteration counts across MPI processes
  bool hasSynced() const { return _has_synced; }

  template <typename Context>
  friend void dataStore(std::ostream &, SolutionInvalidity &, Context);
  template <typename Context>
  friend void dataLoad(std::istream &, SolutionInvalidity &, Context);

private:
  /// Mutex for locking access to the invalid counts
  /// TODO: These can be changed to shared_mutexes
  mutable std::mutex _invalid_mutex;

  typedef VariadicTable<std::string,
                        unsigned long int,
                        unsigned long int,
                        unsigned long int,
                        std::string>
      FullTable;

  /// Build a VariadicTable for solution invalidity
  FullTable summaryTable() const;

  typedef VariadicTable<std::string, std::string, unsigned long int, unsigned long int> TimeTable;

  /// Build a VariadicTable for solution invalidity history
  TimeTable transientTable(unsigned int & time_interval) const;

  /// Create a registry to keep track of the names and occurrences of the solution invalidity
  SolutionInvalidityRegistry & _solution_invalidity_registry;

  /// Store the solution invalidity counts
  std::vector<InvalidCounts> _counts;

  /// Whether or not we've synced (can check counts/existance of warnings or errors)
  bool _has_synced;
  /// Whether or not we have a warning (only after a sync)
  bool _has_solution_warning;
  /// Whether or not we have an invalid solution (only after a sync)
  bool _has_solution_error;
  /// Whether or not we have ever had any warning or solution issue during the simulation
  bool _has_recorded_issue;
};

// datastore and dataload for recover
template <typename Context>
void dataStore(std::ostream & stream,
               SolutionInvalidity::TimestepCounts & timestep_counts,
               Context context);
template <typename Context>
void dataLoad(std::istream & stream,
              SolutionInvalidity::TimestepCounts & timestep_counts,
              Context context);

template <typename Context>
void dataStore(std::ostream & stream, SolutionInvalidity & solution_invalidity, Context context);
template <typename Context>
void dataLoad(std::istream & stream, SolutionInvalidity & solution_invalidity, Context context);
