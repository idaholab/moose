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

class ConservedPassiveScalarMaterial : public Material
{
public:
  static InputParameters validParams();

  ConservedPassiveScalarMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  const ADVariableValue & _rho_passive_var;
  ADMaterialProperty<Real> & _passive_var_prop;
  const ADMaterialProperty<Real> & _rho;
};
