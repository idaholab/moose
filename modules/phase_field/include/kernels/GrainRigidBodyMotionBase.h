/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINRIGIDBODYMOTIONBASE_H
#define GRAINRIGIDBODYMOTIONBASE_H

#include "Kernel.h"

//Forward Declarations
class GrainRigidBodyMotionBase;

template<>
InputParameters validParams<GrainRigidBodyMotionBase>();

class GrainRigidBodyMotionBase : public Kernel
{
public:
  GrainRigidBodyMotionBase(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() { return 0.0; }
  virtual Real computeQpJacobian() { return 0.0; }
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) { return 0.0; }

  /// int label for the Concentration
  unsigned int _c_var;

  /// Variable value for the concentration
  VariableValue & _c;

  /// Variable gradient for the concentration
  VariableGradient & _grad_c;

  unsigned int _ncrys;
  /// Variable value for the order parameters
  std::vector<VariableValue *> _vals;
  std::vector<unsigned int> _vals_var;

  /// Material property for advection velocities
  const MaterialProperty<std::vector<RealGradient> > & _velocity_advection;
  /// Material property for divergence of advection velocities
  const MaterialProperty<std::vector<Real> > & _div_velocity_advection;

  /// Material property for  dervative of advection velocities
  const MaterialProperty<std::vector<RealGradient> > & _velocity_advection_derivative_c;
  /// Material property for dirivative of divergence of advection velocities
  const MaterialProperty<std::vector<Real> > & _div_velocity_advection_derivative_c;
  /// Material property for  dervative of advection velocities
  const MaterialProperty<std::vector<RealGradient> > & _velocity_advection_derivative_eta;
};

#endif //GRAINRIGIDBODYMOTIONBASE_H
