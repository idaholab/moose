//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  /// Reset the number of solution invalid occurrences back to zero for the current time step
  void resetSolutionInvalidTimeStep();

  /// Reset the number of solution invalid occurrences back to zero
  void resetSolutionInvalidCurrentIteration();

  /// Pass the number of solution invalid occurrences from current iteration to cumulative counters
  void solutionInvalidAccumulation();

  /// Pass the number of solution invalid occurrences from current iteration to cumulative time iteration counters
  void solutionInvalidAccumulationTimeStep();

  /// Struct used in _counts for storing invalid occurrences
  struct InvalidCounts
  {
    unsigned int counts = 0;
    unsigned int timestep_counts = 0;
    unsigned int total_counts = 0;
  };

  /// Access the private solution invalidity counts
  const std::vector<InvalidCounts> & counts() const { return _counts; }

  /**
   * Print the summary table of Solution Invalid warnings
   * @param console The output stream to output to
   */
  void print(const ConsoleStream & console) const;

  /**
   * Immediately print the section and message for debug purpose
   */
  void printDebug(InvalidSolutionID _invalid_solution_id) const;

  void sync();

  friend void dataStore(std::ostream &, SolutionInvalidity &, void *);
  friend void dataLoad(std::istream &, SolutionInvalidity &, void *);

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
};

// datastore and dataload for recover
void dataStore(std::ostream & stream, SolutionInvalidity & solution_invalidity, void * context);
void dataLoad(std::istream & stream, SolutionInvalidity & solution_invalidity, void * context);
