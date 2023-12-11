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
 * Computes a time step size based on pseudo-timestep continuation
 */
class PseudoTimestep : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  PseudoTimestep(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  using Postprocessor::getValue;
  virtual Real getValue() const override;

  /// Enum class containing the different options for selecting the timestep
  /// size for pseudo-transient simulations
  enum class PseudotimeMethod
  {
    SER,
    RDM,
    EXP
  };

protected:
  /// The timestep selection method.
  const PseudotimeMethod _method;

  /// Required parameters for the pseudotimestepper
  const Real _initial_dt;
  const Real _alpha;

  /// Number of iterations over which the residual can be lagged
  unsigned int _iterations_window;

  /// Boolean to check if an upper bound was provided
  bool _bound;

  /// Upper bound on timestep
  Real _max_dt;

  /// Transient pseudotimestep
  Real _dt;

  /// arrays for storing residual and iterations sequence
  std::vector<Real> & _residual_norms_sequence;
  std::vector<Real> & _iterations_step_sequence;

  /// Computes the norm of the non-time dependent residual
  Real currentResidualNorm() const;

  /// implementation of SER method
  Real timestepSER() const;

  /// implementation of EXP method
  Real timestepEXP() const;

  /// implementation of RDM method
  Real timestepRDM() const;

  /// Outputs the status of the residual and timestep at the end of time step
  void outputPseudoTimestep(Real curr_dt) const;
};
