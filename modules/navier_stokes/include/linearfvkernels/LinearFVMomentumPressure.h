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

/**
 * Kernel that adds the component of the pressure gradient in the momentum
 * equations to the right hand side.
 *
 * The gradient is the field registered by this kernel. Rhie-Chow can query the kernel for this
 * field so the momentum predictor and H/A construction use the same pressure gradient.
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

  /// Pressure gradient field used by this kernel.
  const LinearFVGradientField & pressureGradientField() const { return _pressure_gradient_field; }

  /// Variable number of the pressure-like variable used by this kernel.
  unsigned int pressureVariableNumber() const { return _pressure_var.number(); }

protected:
  MooseLinearVariableFV<Real> & getPressureVariable(const std::string & vname);

  /// Register the pressure gradient field used by this kernel.
  const LinearFVGradientField & registerPressureGradientField();

  /// Index x|y|z of the momentum equation component
  const unsigned int _index;

  /// Pointer to the linear finite volume pressure variable
  MooseLinearVariableFV<Real> & _pressure_var;

  /// Pressure gradient field used by this kernel.
  const LinearFVGradientField & _pressure_gradient_field;
};
