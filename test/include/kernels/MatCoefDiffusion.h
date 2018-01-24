//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATCOEFDIFFUSION_H
#define MATCOEFDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class MatCoefDiffusion;

template <>
InputParameters validParams<MatCoefDiffusion>();

/**
 * A test class for checking the operation for BlockRestrictable::hasMaterialProperty
 */
class MatCoefDiffusion : public Kernel
{
public:
  MatCoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  std::string _prop_name;
  const MaterialProperty<Real> * _coef;
};

#endif // MATCOEFDIFFUSION_H
