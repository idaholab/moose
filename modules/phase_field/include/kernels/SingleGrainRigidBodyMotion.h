/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SINGLEGRAINRIGIDBODYMOTION_H
#define SINGLEGRAINRIGIDBODYMOTION_H

#include "Kernel.h"

//Forward Declarations
class SingleGrainRigidBodyMotion;

template<>
InputParameters validParams<SingleGrainRigidBodyMotion>();

class SingleGrainRigidBodyMotion : public Kernel
{
public:
  SingleGrainRigidBodyMotion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// Grain number for the kernel to be applied
  unsigned int _op_index;
  /// Material property giving the advection velocity of grains
  const MaterialProperty<std::vector<RealGradient> > & _velocity_advection;
};

#endif //SINGLEGRAINRIGIDBODYMOTION_H
