/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
