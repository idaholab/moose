#include "RhoEVaporMixtureFromPressureTemperatureVelocityIC.h"

registerMooseObject("THMApp", RhoEVaporMixtureFromPressureTemperatureVelocityIC);

template <>
InputParameters
validParams<RhoEVaporMixtureFromPressureTemperatureVelocityIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params += validParams<VaporMixtureInterface<>>();

  params.addClassDescription("Computes the total energy density of a vapor mixture from pressure, "
                             "temperature, and velocity.");

  params.addRequiredCoupledVar("p", "Pressure of the mixture");
  params.addRequiredCoupledVar("T", "Temperature of the mixture");
  params.addRequiredCoupledVar("vel", "Velocity of the mixture");

  return params;
}

RhoEVaporMixtureFromPressureTemperatureVelocityIC::
    RhoEVaporMixtureFromPressureTemperatureVelocityIC(const InputParameters & parameters)
  : VaporMixtureInterface<InitialCondition>(parameters),
    _p(coupledValue("p")),
    _T(coupledValue("T")),
    _vel(coupledValue("vel"))
{
}

Real
RhoEVaporMixtureFromPressureTemperatureVelocityIC::value(const Point & /*p*/)
{
  const auto x = getMassFractionVector();
  const Real rho = _fp_vapor_mixture.rho_from_p_T(_p[_qp], _T[_qp], x);
  const Real e = _fp_vapor_mixture.e_from_p_T(_p[_qp], _T[_qp], x);
  const Real E = e + 0.5 * _vel[_qp] * _vel[_qp];

  return rho * E;
}
