//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SOLVEOBJECT_H
#define SOLVEOBJECT_H

#include "MooseObject.h"
#include "PerfGraphInterface.h"

class SolveObject;
class Executioner;
class FEProblemBase;
class NonlinearSystemBase;

class SolveObject : public MooseObject, public PerfGraphInterface
{
public:
  SolveObject(Executioner * ex);

  /**
   * Solve routine provided by this object.
   * @return True if solver is converged.
   */
  virtual bool solve() = 0;

protected:
  /// Executioner used to construct this
  Executioner & _executioner;
  /// Reference to FEProblem
  FEProblemBase & _problem;
  /// Reference to nonlinear system base for faster access
  NonlinearSystemBase & _nl;
};
#endif // SOLVEOBJECT_H
