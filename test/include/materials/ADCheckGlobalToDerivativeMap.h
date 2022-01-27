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
 * Test object for verifying that \p globalDofIndexToDerivative generates a correct mapping from
 * global dof indices to derivatives for Jacobians
 */
class ADCheckGlobalToDerivativeMap : public Material
{
public:
  static InputParameters validParams();

  ADCheckGlobalToDerivativeMap(const InputParameters & parameters);

  void computeProperties() override;

protected:
  virtual void computeQpProperties() override;

  ADMaterialProperty<Real> & _mat_prop;

  const ADVariableValue & _u;
  const ADVariableValue & _v;
  const MooseVariable & _u_var;
  const MooseVariable & _v_var;
};
