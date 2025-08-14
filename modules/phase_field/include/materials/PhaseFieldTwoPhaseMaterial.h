//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

#define usingPhaseFieldTwoPhaseMateriallMembers usingMaterialMembers;

class PhaseFieldTwoPhaseMaterial : public ADMaterial
{
public:
  static InputParameters validParams();
  PhaseFieldTwoPhaseMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const ADVariableValue & _pf;
  const Real _prop_value_1;
  const Real _prop_value_2;
  ADMaterialProperty<Real> & _prop;

};
