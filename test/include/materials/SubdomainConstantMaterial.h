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
 * Simple material with subdomain-wise constant properties.
 */
class SubdomainConstantMaterial : public Material
{
public:
  static InputParameters validParams();

  SubdomainConstantMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  const MaterialPropertyName & _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;
  std::map<SubdomainID, Real> _mapped_values;
};
