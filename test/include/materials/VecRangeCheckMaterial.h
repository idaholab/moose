//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECRANGECHECKMATERIAL_H
#define VECRANGECHECKMATERIAL_H

#include "Material.h"
#include "MaterialProperty.h"

// Forward Declarations
class VecRangeCheckMaterial;

template <>
InputParameters validParams<VecRangeCheckMaterial>();

/**
 * Simple material to test vector parameter range checking.
 */
class VecRangeCheckMaterial : public Material
{
public:
  VecRangeCheckMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties();
};

#endif // VECRANGECHECKMATERIAL_H
