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
#include "GeneralPostprocessor.h"

/**
 * Computes a time step size based on user-specified CFL number
 */
class PseudoTimestep : public GeneralPostprocessor
{
public:
  PseudoTimestep(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  using Postprocessor::getValue;
  virtual Real getValue() const override;

protected:
  const MooseEnum _method;

  const Real _initial_dt;
  const Real _alpha;
  const Real _max_dt;
  const unsigned int _iterations_window;
  Real _dt;

  std::vector<Real> _residual_norms_sequence;
  std::vector<Real> _iterations_step_sequence;

  Real current_residual_norm() const;
  Real timestep_SER();
  Real timestep_EXP();
  Real timestep_RDM();

  void output_pseudo_timestep(Real & curr_dt, const ExecFlagType & exec_type);

public:
  static InputParameters validParams();
};
