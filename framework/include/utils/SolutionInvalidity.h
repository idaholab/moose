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

// System Includes
#include <array>
#include <atomic>
#include <thread>
#include <future>
#include <mutex>

// Forward Declarations

/**
 * The SolutionInvalidity will contains all the solution invalid warnings info
 */
class SolutionInvalidity
{
public:
  using SolutionInvalidityRegistry = moose::internal::SolutionInvalidityRegistry;

  /// Count solution invalid occurrences for each solution id
  void setSolutionInvalid(SolutionID _solution_id);

  /// Loop over all the tracked objects and determine whether solution invalid is detected
  bool solutionInvalid() const;

  /// Reset the number of solution invalid occurrences back to zero
  void resetSolutionInvalid();

  /// Vector that contains the number of the solution invalid occurrences
  std::vector<unsigned int> _solution_invalid_counts;
};
