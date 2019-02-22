//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FEPROBLEMSOLVE_H
#define FEPROBLEMSOLVE_H

#include "SolveObject.h"

class FEProblemSolve;

template <>
InputParameters validParams<FEProblemSolve>();

class FEProblemSolve : public SolveObject
{
public:
  FEProblemSolve(Executioner * ex);

  /**
   * Picard solve the FEProblem.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

  virtual void setInnerSolve(SolveObject &) override
  {
    mooseError("Cannot set inner solve for FEProblemSolve");
  }

protected:
  /// Splitting
  std::vector<std::string> _splitting;
};
#endif // FEPROBLEMSOLVE_H
