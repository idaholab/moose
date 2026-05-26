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

class LinearFVGradientField;
class RhieChowMassFlux;

/**
 * Kernel that adds the component of the pressure gradient in the momentum
 * equations to the right hand side.
 *
 * By default the gradient is the Green-Gauss gradient stored by the pressure variable. When a
 * RhieChowMassFlux object is supplied, the kernel asks that object for the gradient instead, which
 * lets SIMPLE use the same Green-Gauss/reconstructed pressure-gradient choice in the momentum
 * predictor and in the Rhie-Chow H/A construction.
 */
class LinearFVMomentumPressure : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVMomentumPressure(const InputParameters & params);

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

protected:
  MooseLinearVariableFV<Real> & getPressureVariable(const std::string & vname);

  /// Index x|y|z of the momentum equation component
  const unsigned int _index;

  /// Pointer to the linear finite volume pressure variable
  MooseLinearVariableFV<Real> & _pressure_var;

  /// Pressure gradient field used when no Rhie-Chow object supplies one.
  const LinearFVGradientField & _pressure_gradient_field;

  /// Optional Rhie-Chow object supplying alternate pressure gradients
  const RhieChowMassFlux * const _rhie_chow_mass_flux;
};
