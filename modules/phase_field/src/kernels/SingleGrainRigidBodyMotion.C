/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SingleGrainRigidBodyMotion.h"

template<>
InputParameters validParams<SingleGrainRigidBodyMotion>()
{
  InputParameters params = validParams<GrainRigidBodyMotionBase>();
  params.addClassDescription("Adds rigid mody motion to a single grain");
  params.addParam<unsigned int>("op_index", 0, "Grain number for the kernel to be applied");
  return params;
}

SingleGrainRigidBodyMotion::SingleGrainRigidBodyMotion(const InputParameters & parameters) :
    GrainRigidBodyMotionBase(parameters),
    _op_index(getParam<unsigned int>("op_index"))
{
}

Real
SingleGrainRigidBodyMotion::computeQpResidual()
{
  return   _velocity_advection[_qp][_op_index] * _grad_u[_qp] * _test[_i][_qp]
         + _div_velocity_advection[_qp][_op_index] * _u[_qp] * _test[_i][_qp];
}

Real
SingleGrainRigidBodyMotion::computeQpJacobian()
{
  return   _velocity_advection[_qp][_op_index] * _grad_phi[_j][_qp] * _test[_i][_qp]
         + _velocity_advection_derivative_eta[_qp][_op_index] * _grad_u[_qp] * _phi[_j][_qp] *  _test[_i][_qp]
         + _div_velocity_advection[_qp][_op_index] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
SingleGrainRigidBodyMotion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return   _velocity_advection_derivative_c[_qp][_op_index] * _grad_u[_qp] * _phi[_j][_qp] * _test[_i][_qp]
           + _div_velocity_advection_derivative_c[_qp][_op_index] * _u[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    if (i != _op_index)
    {
      if (jvar == _vals_var[i])
        return _velocity_advection_derivative_eta[_qp][_op_index] * _grad_u[_qp] * _phi[_j][_qp] * _test[_i][_qp];
    }
  }

  return 0.0;
}
