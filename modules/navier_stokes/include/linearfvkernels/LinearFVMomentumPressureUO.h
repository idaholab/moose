//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"

class RhieChowMassFlux;

/**
 * Pressure gradient term for the momentum equation using Rhie-Chow data.
 * This kernel uses the Rhie-Chow user object to supply a pressure gradient
 * consistent with baffle jumps and uses porosity to scale the contribution.
 */
class LinearFVMomentumPressureUO : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();
  LinearFVMomentumPressureUO(const InputParameters & params);

  virtual Real computeMatrixContribution() override;
  virtual Real computeRightHandSideContribution() override;

protected:
  /// Index x|y|z
  const unsigned int _index;

  /// Rhie-Chow user object for pressure gradients
  const RhieChowMassFlux & _rc_uo;

  /// Porosity functor
  const Moose::Functor<Real> & _eps;

  /// Whether to use the corrected (baffle-adjusted) pressure gradient
  const bool _use_corrected_gradient;
};
