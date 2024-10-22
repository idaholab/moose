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
#include "LinearFixedPointSolve.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Executioner to solve multiple linear systems in a fixed point manner
 */
class LinearFixedPointSteady : public Executioner
{
public:
  static InputParameters validParams();

  LinearFixedPointSteady(const InputParameters & parameters);

  void init() override;
  void execute() override;
  bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  bool solveSystem(const unsigned int sys_number, const Moose::PetscSupport::PetscOptions * po);

  /// The solve object
  LinearFixedPointSolve _solve;

  /// Time-related variables
  Real _system_time;
  int & _time_step;
  Real & _time;

private:
  /// Flag to check if the last solve converged or not
  bool _last_solve_converged;

};
