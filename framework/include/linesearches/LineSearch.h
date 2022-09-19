//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"

class FEProblem;
class LineSearch : public MooseObject
{
public:
  static InputParameters validParams();

  LineSearch(const InputParameters & parameters);

  /**
   * zeros the nonlinear iteration count
   */
  void zeroIts() { _nl_its = 0; }

  /**
   * read-only reference to number of non-linear iterations
   */
  size_t nlIts() const { return _nl_its; }

  /**
   * The method that actually implements the line-search
   */
  virtual void lineSearch() { mooseError("You must implement a line-search method."); }

  /**
   * generic setup function that will forward to the virtual interfaces based on the execution flag
   */
  void setup(const ExecFlagType & exec_type);

protected:
  virtual void timestepSetup() {}
  virtual void customSetup(const ExecFlagType & /*exec_type*/) {}
  virtual void initialSetup() {}

  /// Reference to the finite element problem
  FEProblem & _fe_problem;

  /// number of non-linear iterations
  size_t _nl_its;
};

inline void
LineSearch::setup(const ExecFlagType & exec_type)
{
  if (exec_type == EXEC_INITIAL)
    initialSetup();
  else if (exec_type == EXEC_TIMESTEP_BEGIN)
    timestepSetup();
  else
    customSetup(exec_type);
}
