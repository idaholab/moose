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
  MultiGrainRigidBodyMotion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeQpCJacobian();

  /// int label for the Concentration
  unsigned int _c_var;

  /// Variable value for the concentration
  VariableValue & _c;

  /// Variable gradient for the concentration
  VariableGradient & _grad_c;

  /// Material property for advection velocities
  const MaterialProperty<std::vector<RealGradient> > & _velocity_advection;
  /// Material property for divergence of advection velocities
  const MaterialProperty<std::vector<Real> > & _div_velocity_advection;

  /// Material property for  dervative of advection velocities
  const MaterialProperty<std::vector<RealGradient> > & _velocity_advection_derivative;
  /// Material property for dirivative of divergence of advection velocities
  const MaterialProperty<std::vector<Real> > & _div_velocity_advection_derivative;
};

#endif //MULTIGRAINRIGIDBODYMOTION_H
