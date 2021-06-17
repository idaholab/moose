//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"
#include "MaterialProperty.h"

/**
 * Interface material calculates a variable's jump value across an interface
 */
class JumpInterfaceMaterial : public InterfaceMaterial
{
public:
  static InputParameters validParams();

  JumpInterfaceMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const VariableValue & _value;
  const VariableValue & _neighbor_value;
  const ADVariableValue & _ad_value;
  const ADVariableValue & _ad_neighbor_value;
  MaterialProperty<Real> & _jump;
  ADMaterialProperty<Real> & _ad_jump;
};
