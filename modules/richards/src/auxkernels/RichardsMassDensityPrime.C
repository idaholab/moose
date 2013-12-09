//  This post processor returns the derivative of mass wrt to pressure in a region.
//
#include "RichardsMassDensityPrime.h"

template<>
InputParameters validParams<RichardsMassDensityPrime>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<UserObjectName>("sat_UO", "Name of user object that defines saturation");
  params.addRequiredCoupledVar("pressure_var", "The variable that represents the pressure");
  params.addRequiredParam<Real>("p_air", "Air pressure.  Typical value=101E3");
  params.addClassDescription("auxiliary variable which is d(density*saturation)/dP");
  return params;
}

RichardsMassDensityPrime::RichardsMassDensityPrime(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _pressure_var(coupledValue("pressure_var")),
  _density_UO(getUserObject<RichardsDensity>("density_UO")),
  _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
  _sat_UO(getUserObject<RichardsSat>("sat_UO")),
  _p_air(getParam<Real>("p_air"))
{}


Real
RichardsMassDensityPrime::computeValue()
{
  Real density = _density_UO.density(_pressure_var[_qp]);
  Real ddensity_dp = _density_UO.ddensity(_pressure_var[_qp]);

  Real pc = _p_air - _pressure_var[_qp];
  Real seff = _seff_UO.seff(pc);
  Real dseff_dp = -_seff_UO.dseff(pc); // remember dPc/dP = -1

  Real sat = _sat_UO.sat(seff);
  Real dsat_dseff = _sat_UO.dsat(seff);

  return ddensity_dp*sat + density*dsat_dseff*dseff_dp;
}
