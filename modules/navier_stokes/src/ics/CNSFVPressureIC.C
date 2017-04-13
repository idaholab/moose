/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVPressureIC.h"

template <>
InputParameters
validParams<CNSFVPressureIC>()
{
  InputParameters params = validParams<InitialCondition>();

  params.addClassDescription(
      "An initial condition object for computing pressure from conserved variables.");

  params.addRequiredCoupledVar("rho", "Conserved variable: rho");

  params.addRequiredCoupledVar("rhou", "Conserved variable: rhou");

  params.addCoupledVar("rhov", "Conserved variable: rhov");

  params.addCoupledVar("rhow", "Conserved variable: rhow");

  params.addRequiredCoupledVar("rhoe", "Conserved variable: rhoe");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");
  return params;
}

CNSFVPressureIC::CNSFVPressureIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhov(isCoupled("rhov") ? coupledValue("rhov") : _zero),
    _rhow(isCoupled("rhow") ? coupledValue("rhow") : _zero),
    _rhoe(coupledValue("rhoe")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

CNSFVPressureIC::~CNSFVPressureIC() {}

Real
CNSFVPressureIC::value(const Point & /*p*/)
{
  Real v = 1. / _rho[_qp];

  Real e =
      _rhoe[_qp] * v -
      0.5 * v * v * (_rhou[_qp] * _rhou[_qp] + _rhov[_qp] * _rhov[_qp] + _rhow[_qp] * _rhow[_qp]);

  return _fp.pressure(v, e);
}
