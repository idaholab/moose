//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef USECOMPUTEFALSEMATERIAL_H
#define USECOMPUTEFALSEMATERIAL_H

#include "Material.h"
#include "ComputeFalseMaterial.h"

// Forward declaration
class UseComputeFalseMaterial;

template <>
InputParameters validParams<UseComputeFalseMaterial>();

/**
 * UseComputeFalseMaterial instructs a compute=false material
 * called _compute_false_material to compute something using
 * its computeQpThings method.
 * UseComputeFalseMaterial does not actually use the results
 * of that calculation.
 */
class UseComputeFalseMaterial : public Material
{
public:
  UseComputeFalseMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  ComputeFalseMaterial & _compute_false_material;
};

#endif // USECOMPUTEFALSEMATERIAL_H
