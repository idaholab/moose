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

// Forward Declarations
template <class... Ts>
class VariadicTable;

/**
 * The SolutionInvalidity will contains all the solution invalid warnings info
 */
class SolutionInvalidity : protected ConsoleStreamInterface
{
public:
  using SolutionInvalidityRegistry = moose::internal::SolutionInvalidityRegistry;

  /**
   * Create a new SolutionInvalidity
   */
  SolutionInvalidity(MooseApp & app);

  /**
   * Destructor
   */
  ~SolutionInvalidity();

  /// Count solution invalid occurrences for each solution id
  void setSolutionInvalid(SolutionID _solution_id);

  /// Loop over all the tracked objects and determine whether solution invalid is detected
  bool solutionInvalid() const;

  /// Reset the number of solution invalid occurrences back to zero
  void resetSolutionInvalid();

  /// Vector that contains the number of the solution invalid occurrences
  std::vector<unsigned int> _solution_invalid_counts;

  /// @param console The output stream to output to
  void print(const ConsoleStream & console);

  /// The SolutionInvalidityRegistry
  SolutionInvalidityRegistry & _solution_invalidity_registry;

protected:
  typedef VariadicTable<std::string, unsigned long int> FullTable;

private:
  FullTable summaryTable();
};
