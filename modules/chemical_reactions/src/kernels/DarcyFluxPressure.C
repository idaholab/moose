/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DarcyFluxPressure.h"

template <>
InputParameters
validParams<DarcyFluxPressure>()
{
  InputParameters params = validParams<Kernel>();
  RealVectorValue g(0, 0, 0);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector (default is (0, 0, 0))");
  params.addClassDescription("");
  return params;
}

DarcyFluxPressure::DarcyFluxPressure(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _cond(getMaterialProperty<Real>("conductivity")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _density(getDefaultMaterialProperty<Real>("density"))
{
}

Real
DarcyFluxPressure::computeQpResidual()
{
  return _grad_test[_i][_qp] * _cond[_qp] * (_grad_u[_qp] - _density[_qp] * _gravity);
}

Real
DarcyFluxPressure::computeQpJacobian()
{
  return _grad_test[_i][_qp] * _cond[_qp] * _grad_phi[_j][_qp];
}
