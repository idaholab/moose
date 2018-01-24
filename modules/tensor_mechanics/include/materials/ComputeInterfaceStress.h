//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEINTERFACESTRESS_H
#define COMPUTEINTERFACESTRESS_H

#include "Material.h"

class ComputeInterfaceStress;
class RankTwoTensor;

template <>
InputParameters validParams<ComputeInterfaceStress>();

/**
 * Calculates an Extra-Stress tensor that lies in the plane of an interface
 * defined by the gradient of an order parameter.
 */
class ComputeInterfaceStress : public Material
{
public:
  ComputeInterfaceStress(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const VariableGradient & _grad_v;
  const Real _stress;

  MaterialProperty<RankTwoTensor> & _planar_stress;
};

#endif // COMPUTEINTERFACESTRESS_H
