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
#include "FEProblemBase.h"

// Forward declarations
class MooseObject;

#define flagInvalidSolution1(message)                                                              \
  do                                                                                               \
  {                                                                                                \
    static const auto __invalid_id = this->registerInvalidSolutionInternal(message);               \
    this->flagInvalidSolutionInternal(__invalid_id);                                               \
  } while (0)

#define flagInvalidSolution2(prefix, message)                                                      \
  do                                                                                               \
  {                                                                                                \
    static const auto __invalid_id = this->registerInvalidSolutionInternal(message, prefix);       \
    this->flagInvalidSolutionInternal(__invalid_id);                                               \
  } while (0)

#define get_mymacro(_1, _2, NAME, ...) NAME
#define flagInvalidSolution(...)                                                                   \
  get_mymacro(__VA_ARGS__, flagInvalidSolution2, flagInvalidSolution1)(__VA_ARGS__)

/**
 * An interface that allows the marking of invalid solutions during a solve
 */
class SolutionInvalidInterface
{
public:
  SolutionInvalidInterface(MooseObject * const moose_object);

  // SolutionInvalidInterface(MooseObject * const moose_object, const std::string & prefix);

protected:
  void flagInvalidSolutionInternal(InvalidSolutionID _invalid_solution_id);

  InvalidSolutionID registerInvalidSolutionInternal(const std::string & message) const;

  InvalidSolutionID registerInvalidSolutionInternal(const std::string & message,
                                                    const std::string & prefix) const;

private:
  /// The MooseObject that owns this interface
  MooseObject & _si_moose_object;

  /// A reference to FEProblem base
  FEProblemBase & _si_problem;
};
