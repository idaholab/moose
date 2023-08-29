/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
