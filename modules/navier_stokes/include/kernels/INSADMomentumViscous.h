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
 * contributions for the viscous term of the incompressible Navier-Stokes momentum
 * equation.
 */
class INSADMomentumViscous : public ADVectorKernel
{
public:
  static InputParameters validParams();

  INSADMomentumViscous(const InputParameters & parameters);

protected:
  void computeResidual() override;
  void computeResidualsForJacobian() override;
  ADReal computeQpResidual() override;

  /**
   * Computes the cartesian coordinate system viscous term
   */
  ADRealTensorValue qpViscousTerm();

  /**
   * Computes an additional contribution to the viscous term present of RZ coordinate systems
   * (assumes axisymmetric)
   */
  ADRealVectorValue qpAdditionalRZTerm();

  const ADMaterialProperty<Real> & _mu;

  const Moose::CoordinateSystemType & _coord_sys;

  /// Either traction or laplace
  MooseEnum _form;
};
