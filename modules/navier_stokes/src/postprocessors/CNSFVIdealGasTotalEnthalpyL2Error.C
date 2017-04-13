/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVIdealGasTotalEnthalpyL2Error.h"

template <>
InputParameters
validParams<CNSFVIdealGasTotalEnthalpyL2Error>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();

  params.addClassDescription("A PostProcessor object to calculate the L2 error of ideal gas total "
                             "enthalpy for the CNS equations.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  params.addRequiredParam<Real>("infinity_density", "Infinity density");

  params.addRequiredParam<Real>("infinity_x_velocity",
                                "Infinity velocity component in x-direction");

  params.addParam<Real>("infinity_y_velocity", 0., "Infinity velocity component in y-direction");

  params.addParam<Real>("infinity_z_velocity", 0., "Infinity velocity component in z-direction");

  params.addRequiredParam<Real>("infinity_pressure", "Infinity pressure");

  return params;
}

CNSFVIdealGasTotalEnthalpyL2Error::CNSFVIdealGasTotalEnthalpyL2Error(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _inf_rho(getParam<Real>("infinity_density")),
    _inf_uadv(getParam<Real>("infinity_x_velocity")),
    _inf_vadv(getParam<Real>("infinity_y_velocity")),
    _inf_wadv(getParam<Real>("infinity_z_velocity")),
    _inf_pres(getParam<Real>("infinity_pressure")),
    _rho(getMaterialProperty<Real>("rho")),
    _enth(getMaterialProperty<Real>("enthalpy"))
{
}

Real
CNSFVIdealGasTotalEnthalpyL2Error::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
CNSFVIdealGasTotalEnthalpyL2Error::computeQpIntegral()
{
  Real gamma = _fp.gamma(0., 0.);

  Real diff =
      _rho[_qp] * _enth[_qp] - gamma / (gamma - 1.) * _inf_pres -
      0.5 * _inf_rho * (_inf_uadv * _inf_uadv + _inf_vadv * _inf_vadv + _inf_wadv * _inf_wadv);

  return diff * diff;
}
