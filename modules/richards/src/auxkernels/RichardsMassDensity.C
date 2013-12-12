//  This post processor returns the mass value of a region.  It is used
//  to check that mass is conserved
//
#include "RichardsMassDensity.h"

template<>
InputParameters validParams<RichardsMassDensity>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<UserObjectName>("sat_UO", "Name of user object that defines saturation");
  params.addRequiredCoupledVar("pressure_var", "The variable that represents the pressure");
  return params;
}

RichardsMassDensity::RichardsMassDensity(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _pressure_var(coupledValue("pressure_var")),
  _density_UO(getUserObject<RichardsDensity>("density_UO")),
  _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
  _sat_UO(getUserObject<RichardsSat>("sat_UO"))
{}


Real
RichardsMassDensity::computeValue()
{
  Real density = _density_UO.density(_pressure_var[_qp]);
  Real pc = -_pressure_var[_qp];
  Real seff = _seff_UO.seff(pc);
  Real sat = _sat_UO.sat(seff);

  return density*sat;
}
