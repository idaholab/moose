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
  params.addParam<unsigned int>("op_index",0, "Grain number for the kernel to be applied");
  params.addParam<MaterialPropertyName>("advection_velocity", "Material property tgiving the advection velocity of grains");
  return params;
}

SingleGrainRigidBodyMotion::SingleGrainRigidBodyMotion(const std::string & name,
                             InputParameters parameters) :
    Kernel(name, parameters),
    _op_index(getParam<unsigned int>("op_index")),
    _velocity_advection(getMaterialProperty<std::vector<RealGradient> >("advection_velocity"))
{
}

Real
SingleGrainRigidBodyMotion::computeQpResidual()
{
  return _velocity_advection[_qp][_op_index] * _grad_u[_qp] *_test[_i][_qp];
}

Real
SingleGrainRigidBodyMotion::computeQpJacobian()
{
  return _velocity_advection[_qp][_op_index] * _grad_phi[_j][_qp] * _test[_i][_qp];
}
