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

// System Includes
#include <chrono>
#include <memory>

/// Count solution invalid occurrences for each solution id
void
SolutionInvalidity::setSolutionInvalid(SolutionID _solution_id)
{
  if (_solution_invalid_counts.size() <= _solution_id)
  {
    _solution_invalid_counts.resize(_solution_id + 1);
    ++_solution_invalid_counts[_solution_id];
  }
  else
  {
    ++_solution_invalid_counts[_solution_id];
  }
}

/// Loop over all the tracked objects and determine whether solution invalid is detected
bool
SolutionInvalidity::solutionInvalid() const
{
  unsigned int sum_of_elems = 0;
  std::for_each(_solution_invalid_counts.begin(),
                _solution_invalid_counts.end(),
                [&](int n) { sum_of_elems += n; });
  if (sum_of_elems < 1)
  {
    return false;
  }
  else
  {
    return true;
  }
}

/// Reset the number of solution invalid occurrences back to zero
void
SolutionInvalidity::resetSolutionInvalid()
{
  std::fill(_solution_invalid_counts.begin(), _solution_invalid_counts.end(), 0);
}
