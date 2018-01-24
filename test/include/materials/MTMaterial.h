//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MTMATERIAL_H
#define MTMATERIAL_H

#include "Material.h"
#include "MaterialProperty.h"

// Forward Declarations
class MTMaterial;

template <>
InputParameters validParams<MTMaterial>();

/**
 * Simple material with constant properties.
 */
class MTMaterial : public Material
{
public:
  MTMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _mat_prop;
  Real _value;
};

#endif // DIFF1MATERIAL_H
