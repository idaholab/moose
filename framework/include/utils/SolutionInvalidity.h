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
 * The SolutionInvalidity will contains all the solution invalid warnings info
 */
class SolutionInvalidity : protected ConsoleStreamInterface, public ParallelObject
{
public:
  using SolutionInvalidityRegistry = moose::internal::SolutionInvalidityRegistry;

  /**
   * Create a new SolutionInvalidity
   */
  SolutionInvalidity(MooseApp & app);

  /// Count solution invalid occurrences for each solution id
  void flagInvalidSolutionInternal(InvalidSolutionID _invalid_solution_id);

  /// Loop over all the tracked objects and determine whether solution invalid is detected
  bool solutionInvalid() const;

  /// Reset the number of solution invalid occurrences back to zero for each time iteration
  void resetSolutionInvalidTimeIter();

  /// Reset the number of solution invalid occurrences back to zero
  void resetSolutionInvalid();

  /// Pass the number of solution invalid occurrences from current iteration to comulative counters
  void solutionInvalidAccumulation();

  /**
   * Print the summary table of Solution Invalid warnings
   *  @param console The output stream to output to
   */
  void print(const ConsoleStream & console) const;

  /**
   * Immediately print the section and message for debug purpose
   *  @param console The output stream to output to
   */
  void printDebug(InvalidSolutionID _invalid_solution_id) const;

  void sync();

private:
  /// Mutex for locking access to the invalid counts
  /// NOTE: These can be changed to shared_mutexes once we get C++17
  mutable std::mutex _invalid_mutex;
  typedef VariadicTable<std::string,
                        unsigned long int,
                        unsigned long int,
                        unsigned long int,
                        std::string>
      FullTable;

  FullTable summaryTable() const;

  void sync();

  /// The SolutionInvalidityRegistry
  SolutionInvalidityRegistry & _solution_invalidity_registry;

  /// Struct used in _counts for storing invalid occurrences
  struct InvalidCounts
  {
    unsigned int counts;
    unsigned int timeiter_counts;
    unsigned int total_counts;
  };

  std::vector<InvalidCounts> _counts;
};
