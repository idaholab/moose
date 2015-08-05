/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiGrainRigidBodyMotion.h"

template<>
InputParameters validParams<MultiGrainRigidBodyMotion>()
{
  InputParameters params = validParams<GrainRigidBodyMotionBase>();
  params.addClassDescription("Adds rigid mody motion to grains");
  return params;
}

MultiGrainRigidBodyMotion::MultiGrainRigidBodyMotion(const InputParameters & parameters) :
    GrainRigidBodyMotionBase(parameters)
{
}

Real
MultiGrainRigidBodyMotion::computeQpResidual()
{
  RealGradient vadv_total = 0.0;
  Real div_vadv_total = 0.0;
  for (unsigned int i = 0; i < _velocity_advection[_qp].size(); ++i)
  {
    vadv_total += _velocity_advection[_qp][i];
    div_vadv_total += _div_velocity_advection[_qp][i];
  }

  return vadv_total * _grad_c[_qp] * _test[_i][_qp] + div_vadv_total * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeQpJacobian()
{
  if (_c_var == _var.number()) //Requires c jacobian
    return computeCVarJacobianEntry();

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_c_var == jvar) //Requires c jacobian
    return computeCVarJacobianEntry();

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeCVarJacobianEntry()
{
  RealGradient vadv_total = 0.0;
  Real div_vadv_total = 0.0;
  RealGradient dvadvdc_total = 0.0;
  Real ddivvadvdc_total = 0.0;
  for (unsigned int i = 0; i < _velocity_advection[_qp].size(); ++i)
  {
    vadv_total += _velocity_advection[_qp][i];
    div_vadv_total += _div_velocity_advection[_qp][i];
    dvadvdc_total += _velocity_advection_derivative_c[_qp][i];
    ddivvadvdc_total += _div_velocity_advection_derivative_c[_qp][i];
  }

  return  vadv_total * _grad_phi[_j][_qp] * _test[_i][_qp] + dvadvdc_total * _grad_c[_qp] * _phi[_j][_qp] * _test[_i][_qp]
          + div_vadv_total * _phi[_j][_qp] * _test[_i][_qp] + ddivvadvdc_total * _c[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
