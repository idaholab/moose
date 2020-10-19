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

/**
 * A material that optionally couples both a finite element and finite volume variable (strictly
 * speaking they don't have to be one or the either)
 */
class FEFVCouplingMaterial : public Material
{
public:
  static InputParameters validParams();

  FEFVCouplingMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const ADVariableValue & _fe_var;
  const ADVariableValue & _fv_var;
  ADMaterialProperty<Real> * const _fe_prop;
  ADMaterialProperty<Real> * const _fv_prop;
  ADMaterialProperty<Real> * const _declared_prop;
  const ADMaterialProperty<Real> * const _retrieved_prop;
};
