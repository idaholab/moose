//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DOTERRORMATERIAL_H
#define DOTERRORMATERIAL_H

#include "Material.h"

// Forward Declarations
class DotErrorMaterial;

template <>
InputParameters validParams<DotErrorMaterial>();

class DotErrorMaterial : public Material
{
public:
  DotErrorMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const std::string _prop_name;
  MaterialProperty<Real> & _mat_prop;
  const MaterialProperty<Real> & _mat_prop_dot;
};

#endif
