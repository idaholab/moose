//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SteadyBase.h"

/**
 * Steady executioners usually only call "solve()" on the NonlinearSystem once.
 */
class Steady : public SteadyBase
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  Steady(const InputParameters & parameters);

  virtual void init() override;

  /// Check if a time kernel has been declared
  virtual void checkIntegrity();

protected:
  /// The solve object to use in this executioner
  FEProblemSolve _feproblem_solve;
};
