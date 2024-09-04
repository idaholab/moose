//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "SolutionInvalidity.h"

// Forward declarations
class MooseObject;
class FEProblemBase;

#define flagInvalidSolution(message)                                                               \
  do                                                                                               \
  {                                                                                                \
    static const auto __invalid_id = this->registerInvalidSolutionInternal(message, false);        \
    this->flagInvalidSolutionInternal<false>(__invalid_id);                                        \
  } while (0)

#define flagSolutionWarning(message)                                                               \
  do                                                                                               \
  {                                                                                                \
    static const auto __invalid_id = this->registerInvalidSolutionInternal(message, true);         \
    this->flagInvalidSolutionInternal<true>(__invalid_id);                                         \
  } while (0)

/**
 * An interface that allows the marking of invalid solutions during a solve
 */
class SolutionInvalidInterface
{
public:
  SolutionInvalidInterface(MooseObject * const moose_object);

protected:
  template <bool warning>
  void flagInvalidSolutionInternal(const InvalidSolutionID invalid_solution_id) const;

  // Register invalid solution with a message
  InvalidSolutionID registerInvalidSolutionInternal(const std::string & message,
                                                    const bool warning) const;

private:
  /// The MooseObject that owns this interface
  MooseObject & _si_moose_object;

  /// A reference to FEProblem base
  FEProblemBase & _si_problem;
};
