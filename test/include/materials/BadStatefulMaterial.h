//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Material.h"

#ifndef BADSTATEFULMATERIAL_H
#define BADSTATEFULMATERIAL_H

// Forward Declarations
class BadStatefulMaterial;

template <>
InputParameters validParams<BadStatefulMaterial>();

/// Tries to retrieve non-existing old/older versions of a material property.
class BadStatefulMaterial : public Material
{
public:
  BadStatefulMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif // STATEFULMATERIAL_H
