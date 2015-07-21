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
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Adds rigid mody motion to a single grain");
  params.addRequiredCoupledVar("c", "Concentration");
  params.addParam<unsigned int>("op_index",0, "Grain number for the kernel to be applied");
  params.addParam<MaterialPropertyName>("advection_velocity", "Material property tgiving the advection velocity of grains");
  params.addParam<MaterialPropertyName>("advection_velocity_divergence", "Material property for divergence of advection velocities");
  return params;
}

SingleGrainRigidBodyMotion::SingleGrainRigidBodyMotion(const std::string & name,
                             InputParameters parameters) :
    Kernel(name, parameters),
    _c_var(coupled("c")),
    _c(coupledValue("c")),
    _grad_c(coupledGradient("c")),
    _op_index(getParam<unsigned int>("op_index")),
    _velocity_advection(getMaterialProperty<std::vector<RealGradient> >("advection_velocity")),
    _div_velocity_advection(getMaterialProperty<std::vector<Real> >("advection_velocity_divergence")),
    _velocity_advection_derivative(getMaterialProperty<std::vector<RealGradient> >("advection_velocity_derivative")),
    _div_velocity_advection_derivative(getMaterialProperty<std::vector<Real> >("advection_velocity_divergence_derivative"))
{
}

Real
SingleGrainRigidBodyMotion::computeQpResidual()
{
  return _velocity_advection[_qp][_op_index] * _grad_c[_qp] * _test[_i][_qp] + _div_velocity_advection[_qp][_op_index] * _c[_qp] * _test[_i][_qp];
}

Real
SingleGrainRigidBodyMotion::computeQpJacobian()
{
    if (_c_var == _var.number()) //Requires c jacobian
      return computeQpCJacobian();

    return 0.0;
}

Real
SingleGrainRigidBodyMotion::computeQpCJacobian()
{
  return _velocity_advection[_qp][_op_index] * _grad_phi[_j][_qp] * _test[_i][_qp] + _velocity_advection_derivative[_qp][_op_index] * _grad_c[_qp] * _phi[_j][_qp] *  _test[_i][_qp] + _div_velocity_advection[_qp][_op_index] * _phi[_j][_qp] * _test[_i][_qp] + _div_velocity_advection_derivative[_qp][_op_index] * _c[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
