//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the pressure term of the incompressible Navier-Stokes momentum
 * equation.
 */
class INSADMomentumPressure : public ADVectorKernel
{
public:
  static InputParameters validParams();

  INSADMomentumPressure(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Whether to integrate the pressure term by parts
  const bool _integrate_p_by_parts;

  /// The pressure
  const ADVariableValue & _p;

  /// The gradient of the pressure
  const ADVariableGradient & _grad_p;

  /// The coordinate system type, e.g. XYZ, RZ, RSPHERICAL
  const Moose::CoordinateSystemType & _coord_sys;

  /// The porosity
  const VariableValue & _eps;
};
