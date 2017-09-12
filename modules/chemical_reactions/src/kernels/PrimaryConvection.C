/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PrimaryConvection.h"

template <>
InputParameters
validParams<PrimaryConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("p", "Pressure");
  RealVectorValue g(0, 0, 0);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector (default is (0, 0, 0))");
  params.addClassDescription("Convection of primary species");
  return params;
}

PrimaryConvection::PrimaryConvection(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _cond(getMaterialProperty<Real>("conductivity")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _density(getDefaultMaterialProperty<Real>("density")),
    _grad_p(coupledGradient("p")),
    _pvar(coupled("p"))

{
}

Real
PrimaryConvection::computeQpResidual()
{
  RealVectorValue darcy_vel = -_cond[_qp] * (_grad_p[_qp] - _density[_qp] * _gravity);

  return _test[_i][_qp] * (darcy_vel * _grad_u[_qp]);
}

Real
PrimaryConvection::computeQpJacobian()
{
  RealVectorValue darcy_vel = -_cond[_qp] * (_grad_p[_qp] - _density[_qp] * _gravity);

  return _test[_i][_qp] * (darcy_vel * _grad_phi[_j][_qp]);
}

Real
PrimaryConvection::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _pvar)
  {
    RealVectorValue ddarcy_vel_dp = -_cond[_qp] * _grad_phi[_j][_qp];
    return _test[_i][_qp] * (ddarcy_vel_dp * _grad_u[_qp]);
  }
  else
    return 0.0;
}
