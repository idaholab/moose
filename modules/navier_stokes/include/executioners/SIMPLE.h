//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executioner.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Something informative will come here eventually
 */
class SIMPLE : public Executioner
{
public:
  static InputParameters validParams();

  SIMPLE(const InputParameters & parameters);

  void init() override;
  void execute() override;
  bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  FEProblemBase & _problem;

  FEProblemSolve _feproblem_solve;

  Real _system_time;
  int & _time_step;
  Real & _time;

private:
  const unsigned int _momentum_sys_number;
  const unsigned int _pressure_sys_number;

  bool _last_solve_converged;
};
