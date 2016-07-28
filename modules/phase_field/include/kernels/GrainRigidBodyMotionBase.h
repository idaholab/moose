/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINRIGIDBODYMOTIONBASE_H
#define GRAINRIGIDBODYMOTIONBASE_H

#include "Kernel.h"
#include "DerivativeMaterialPropertyNameInterface.h"

//Forward Declarations
class GrainRigidBodyMotionBase;

template<>
InputParameters validParams<GrainRigidBodyMotionBase>();

class GrainRigidBodyMotionBase :
    public Kernel,
    public DerivativeMaterialPropertyNameInterface
{
public:
  GrainRigidBodyMotionBase(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() { return 0.0; }
  virtual Real computeQpJacobian() { return 0.0; }
  virtual Real computeQpOffDiagJacobian(unsigned int /* jvar */ ) { return 0.0; }

  /// int label for the Concentration
  unsigned int _c_var;

  /// Variable value for the concentration
  const VariableValue & _c;

  /// Variable gradient for the concentration
  const VariableGradient & _grad_c;

  VariableName _c_name;
  unsigned int _op_num;
  /// Variable value for the order parameters
  std::vector<const VariableValue *> _vals;
  std::vector<unsigned int> _vals_var;

  /// type of force density material
  std::string _base_name;

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
