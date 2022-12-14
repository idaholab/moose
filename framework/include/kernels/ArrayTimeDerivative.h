//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayTimeKernel.h"

class ArrayTimeDerivative : public ArrayTimeKernel
{
public:
  static InputParameters validParams();

  ArrayTimeDerivative(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;
  virtual RealEigenVector computeQpJacobian() override;
  virtual RealEigenMatrix computeQpOffDiagJacobian(const MooseVariableFEBase & jvar) override;

  /// whether or not the coefficient property is provided
  const bool _has_coefficient;
  /// scalar time derivative coefficient
  const MaterialProperty<Real> * const _coeff;
  /// array time derivative coefficient
  const MaterialProperty<RealEigenVector> * const _coeff_array;
  /// matrix time derivative coefficient
  const MaterialProperty<RealEigenMatrix> * const _coeff_2d_array;
};
