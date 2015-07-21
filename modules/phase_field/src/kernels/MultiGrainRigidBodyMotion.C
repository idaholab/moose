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
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Adds rigid mody motion to grains");
  params.addRequiredCoupledVar("c", "Concentration");
  params.addParam<MaterialPropertyName>("advection_velocity", "Material property for advection velocities");
  params.addParam<MaterialPropertyName>("advection_velocity_divergence", "Material property for divergence of advection velocities");
  return params;
}

MultiGrainRigidBodyMotion::MultiGrainRigidBodyMotion(const std::string & name,
                             InputParameters parameters) :
    Kernel(name, parameters),
    _c_var(coupled("c")),
    _c(coupledValue("c")),
    _grad_c(coupledGradient("c")),
    _velocity_advection(getMaterialProperty<std::vector<RealGradient> >("advection_velocity")),
    _div_velocity_advection(getMaterialProperty<std::vector<Real> >("advection_velocity_divergence")),
    _velocity_advection_derivative(getMaterialProperty<std::vector<RealGradient> >("advection_velocity_derivative")),
    _div_velocity_advection_derivative(getMaterialProperty<std::vector<Real> >("advection_velocity_divergence_derivative"))
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
      return computeQpCJacobian();

    return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_c_var == jvar) //Requires c jacobian
    return computeQpCJacobian();

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpCJacobian()
{
  RealGradient vadv_total = 0.0;
  Real div_vadv_total = 0.0;
  RealGradient dvadvdc_total = 0.0;
  Real ddivvadvdc_total = 0.0;
  for (unsigned int i = 0; i < _velocity_advection[_qp].size(); ++i)
  {
    vadv_total += _velocity_advection[_qp][i];
    div_vadv_total += _div_velocity_advection[_qp][i];
    dvadvdc_total += _velocity_advection_derivative[_qp][i];
    ddivvadvdc_total += _div_velocity_advection_derivative[_qp][i];
  }

  return  vadv_total * _grad_phi[_j][_qp] * _test[_i][_qp] + dvadvdc_total * _grad_c[_qp] * _phi[_j][_qp] * _test[_i][_qp] + div_vadv_total * _phi[_j][_qp] * _test[_i][_qp] + ddivvadvdc_total * _c[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
