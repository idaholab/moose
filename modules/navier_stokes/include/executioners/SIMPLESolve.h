//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

class SIMPLESolve : public SolveObject
{
public:
  SIMPLESolve(Executioner & ex);

  static InputParameters validParams();

  /**
   * Will be the inner solve, so the SIMPLE solve.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

  virtual void setInnerSolve(SolveObject &) override
  {
    mooseError("Cannot set inner solve for SIMPLESolve");
  }
};
