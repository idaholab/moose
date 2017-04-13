/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVMachIC.h"

template <>
InputParameters
validParams<CNSFVMachIC>()
{
  InputParameters params = validParams<InitialCondition>();

  params.addClassDescription(
      "An initial condition object for computing Mach number from conserved variables.");

  params.addRequiredCoupledVar("rho", "Conserved variable: rho");

  params.addRequiredCoupledVar("rhou", "Conserved variable: rhou");

  params.addCoupledVar("rhov", "Conserved variable: rhov");

  params.addCoupledVar("rhow", "Conserved variable: rhow");

  params.addRequiredCoupledVar("rhoe", "Conserved variable: rhoe");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");
  return params;
}

CNSFVMachIC::CNSFVMachIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhov(isCoupled("rhov") ? coupledValue("rhov") : _zero),
    _rhow(isCoupled("rhow") ? coupledValue("rhow") : _zero),
    _rhoe(coupledValue("rhoe")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

CNSFVMachIC::~CNSFVMachIC() {}

Real
CNSFVMachIC::value(const Point & /*p*/)
{
  Real v = 1. / _rho[_qp];

  Real vdov = v * v * (_rhou[_qp] * _rhou[_qp] + _rhov[_qp] * _rhov[_qp] + _rhow[_qp] * _rhow[_qp]);

  Real e = _rhoe[_qp] * v - 0.5 * vdov;

  return std::sqrt(vdov) / _fp.c(v, e);
}
