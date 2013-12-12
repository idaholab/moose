//  This post processor returns the mobility d(density*relperm)/dP
//
#include "RichardsMobilityPrime.h"

template<>
InputParameters validParams<RichardsMobilityPrime>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_var", "The variable that represents the pressure");
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<UserObjectName>("relperm_UO", "Name of user object that defines relative permeability");
  params.addClassDescription("auxillary variable which is d(density*relative_permeability)/dP");
  return params;
}

RichardsMobilityPrime::RichardsMobilityPrime(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _pressure_var(coupledValue("pressure_var")),
  _density_UO(getUserObject<RichardsDensity>("density_UO")),
  _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
  _relperm_UO(getUserObject<RichardsRelPerm>("relperm_UO"))
{}

Real
RichardsMobilityPrime::computeValue()
{
  Real density = _density_UO.density(_pressure_var[_qp]);
  Real ddensity_dp = _density_UO.ddensity(_pressure_var[_qp]);

  Real pc = -_pressure_var[_qp];
  Real seff = _seff_UO.seff(pc);
  Real dseff_dp = -_seff_UO.dseff(pc); // remember dPc/dP = -1

  Real relperm = _relperm_UO.relperm(seff);
  Real drelperm_dseff = _relperm_UO.drelperm(seff);

  return ddensity_dp*relperm + density*drelperm_dseff*dseff_dp;
}
