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
  params.addParam<MaterialPropertyName>("advection_velocity", "Material property to multiply the random numbers with (defaults to 1.0 if omitted)");
  return params;
}

MultiGrainRigidBodyMotion::MultiGrainRigidBodyMotion(const std::string & name,
                             InputParameters parameters) :
    Kernel(name, parameters),
    _velocity_advection(getMaterialProperty<std::vector<RealGradient> >("advection_velocity"))
{
}

Real
MultiGrainRigidBodyMotion::computeQpResidual()
{
  RealGradient _vadv_total = 0.0;
  for (unsigned int i = 0; i < _velocity_advection[_qp].size(); ++i)
    _vadv_total += _velocity_advection[_qp][i];

  return _vadv_total *  _grad_u[_qp] *_test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeQpJacobian()
{
  RealGradient _vadv_total = 0.0;
  for (unsigned int i = 0; i < _velocity_advection[_qp].size(); ++i)
    _vadv_total += _velocity_advection[_qp][i];

  return  _vadv_total * _grad_phi[_j][_qp] * _test[_i][_qp];
}
