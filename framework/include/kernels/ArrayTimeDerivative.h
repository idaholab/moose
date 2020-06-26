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

// Forward Declaration
class ArrayTimeDerivative;

template <>
InputParameters validParams<ArrayTimeDerivative>();

class ArrayTimeDerivative : public ArrayTimeKernel
{
public:
  static InputParameters validParams();

  ArrayTimeDerivative(const InputParameters & parameters);

protected:
  virtual RealEigenVector computeQpResidual() override;
  virtual RealEigenVector computeQpJacobian() override;
  virtual RealEigenMatrix computeQpOffDiagJacobian(MooseVariableFEBase & jvar) override;

  /// scalar time derivative coefficient
  const MaterialProperty<Real> * const _coeff;
  /// array time derivative coefficient
  const MaterialProperty<RealEigenVector> * const _coeff_array;
  /// matrix time derivative coefficient
  const MaterialProperty<RealEigenMatrix> * const _coeff_2d_array;
};
