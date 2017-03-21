/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVIdealGasEntropyL2Error.h"

template <>
InputParameters
validParams<CNSFVIdealGasEntropyL2Error>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();

  params.addClassDescription("A PostProcessor object to calculate the L2 error of ideal gas "
                             "entropy production for the CNS equations.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  params.addRequiredParam<Real>("infinity_density", "Infinity density");

  params.addRequiredParam<Real>("infinity_pressure", "Infinity pressure");

  return params;
}

CNSFVIdealGasEntropyL2Error::CNSFVIdealGasEntropyL2Error(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _inf_rho(getParam<Real>("infinity_density")),
    _inf_pres(getParam<Real>("infinity_pressure")),
    _rho(getMaterialProperty<Real>("rho")),
    _pres(getMaterialProperty<Real>("pressure"))
{
}

Real
CNSFVIdealGasEntropyL2Error::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
CNSFVIdealGasEntropyL2Error::computeQpIntegral()
{
  Real diff = (_pres[_qp] / _inf_pres) * std::pow(_inf_rho / _rho[_qp], _fp.gamma(0., 0.)) - 1.;

  return diff * diff;
}
