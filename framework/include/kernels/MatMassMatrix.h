//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MassMatrixBase.h"

/**
 * Calculates a mass matrix using a material property for density.
 */
class MatMassMatrix : public MassMatrixBase
{
public:
  static InputParameters validParams();

  MatMassMatrix(const InputParameters & parameters);

protected:
  virtual Real computeQpJacobian() override;

  /// The density of the material
  const MaterialProperty<Real> & _density;
};
