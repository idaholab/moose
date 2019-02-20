#include "RhoVaporMixtureFromPressureTemperatureIC.h"

registerMooseObject("THMApp", RhoVaporMixtureFromPressureTemperatureIC);

template <>
InputParameters
validParams<RhoVaporMixtureFromPressureTemperatureIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params += validParams<VaporMixtureInterface<>>();

  params.addClassDescription(
      "Computes the density of a vapor mixture from pressure and temperature.");

  params.addRequiredCoupledVar("p", "Pressure of the mixture");
  params.addRequiredCoupledVar("T", "Temperature of the mixture");

  return params;
}

RhoVaporMixtureFromPressureTemperatureIC::RhoVaporMixtureFromPressureTemperatureIC(
    const InputParameters & parameters)
  : VaporMixtureInterface<InitialCondition>(parameters),
    _p(coupledValue("p")),
    _T(coupledValue("T"))
{
}

Real
RhoVaporMixtureFromPressureTemperatureIC::value(const Point & /*p*/)
{
  const auto x = getMassFractionVector();
  return _fp_vapor_mixture.rho_from_p_T(_p[_qp], _T[_qp], x);
}
