//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"

/**
 * A material used for testing the material derivative test kernel
 */
class MaterialDerivativeTestMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  MaterialDerivativeTestMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// the material property for which derivatives are to be tested
  MaterialProperty<Real> & _p;
  /// derivative of material property with respect to nonlinear variable 1
  MaterialProperty<Real> & _dpdu;
  /// derivative of material property with respect to nonlinear variable 2
  MaterialProperty<Real> & _dpdv;
  /// nonlinear variable 1
  const VariableValue & _u;
  /// nonlinear variable 2
  const VariableValue & _v;
};
