/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVEntropyProductionAux.h"

template <>
InputParameters
validParams<CNSFVEntropyProductionAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("An aux kernel for calculating entropy production.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  params.addRequiredParam<Real>("infinity_density", "Infinity density");

  params.addRequiredParam<Real>("infinity_pressure", "Infinity pressure");

  return params;
}

CNSFVEntropyProductionAux::CNSFVEntropyProductionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _rho(getMaterialProperty<Real>("rho")),
    _pres(getMaterialProperty<Real>("pressure")),
    _inf_rho(getParam<Real>("infinity_density")),
    _inf_pres(getParam<Real>("infinity_pressure"))
{
}

Real
CNSFVEntropyProductionAux::computeValue()
{
  return (_pres[_qp] / _inf_pres) * std::pow(_inf_rho / _rho[_qp], _fp.gamma(0., 0.)) - 1.;
}
