/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTIGRAINRIGIDBODYMOTION_H
#define MULTIGRAINRIGIDBODYMOTION_H

#include "Kernel.h"

//Forward Declarations
class MultiGrainRigidBodyMotion;

template<>
InputParameters validParams<MultiGrainRigidBodyMotion>();

class MultiGrainRigidBodyMotion : public Kernel
{
public:
  MultiGrainRigidBodyMotion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  unsigned int _total_grains;
  const MaterialProperty<std::vector<RealGradient> > & _velocity_advection;
};

#endif //MULTIGRAINRIGIDBODYMOTION_H
