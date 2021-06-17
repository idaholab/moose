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

class QuadratureMaterial : public Material
{
public:
  static InputParameters validParams();

  QuadratureMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const std::string _prop_name;
  MaterialProperty<Real> & _mat_prop;
};
