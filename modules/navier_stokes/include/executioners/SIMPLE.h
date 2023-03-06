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
#include "INSFVRhieChowInterpolatorSegregated.h"

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

  /// The number of the nonlinear system corresponding to the momentum equation
  const unsigned int _momentum_sys_number;
  /// The number of the nonlinear system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;
  /// Reference to the nonlinear system corresponding to the momentum equation
  NonlinearSystemBase & _momentum_sys;
  /// Reference to the nonlinear system corresponding to the pressure equation
  NonlinearSystemBase & _pressure_sys;

  INSFVRhieChowInterpolatorSegregated * _rc_uo;

private:
  bool _last_solve_converged;
  const std::string _momentum_tag_name;
  const TagID _momentum_tag_id;

  const Real _momentum_variable_relaxation;
  const Real _pressure_variable_relaxation;

  const Real _momentum_absolute_tolerance;
  const Real _pressure_absolute_tolerance;

  const unsigned int _num_iterations;
};
