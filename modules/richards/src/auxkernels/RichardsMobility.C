//  This post processor returns the mobility (density*relperm)
//
#include "RichardsMobility.h"

template<>
InputParameters validParams<RichardsMobility>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_var", "The variable that represents the pressure");
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<UserObjectName>("relperm_UO", "Name of user object that defines relative permeability");
  params.addRequiredParam<Real>("p_air", "Air pressure.  Typical value=101E3");
  return params;
}

RichardsMobility::RichardsMobility(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _pressure_var(coupledValue("pressure_var")),
  _density_UO(getUserObject<RichardsDensity>("density_UO")),
  _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
  _relperm_UO(getUserObject<RichardsRelPerm>("relperm_UO")),
  _p_air(getParam<Real>("p_air"))
{}


Real
RichardsMobility::computeValue()
{
  Real density = _density_UO.density(_pressure_var[_qp]);

  Real pc = _p_air - _pressure_var[_qp];
  Real seff = _seff_UO.seff(pc);

  Real relperm = _relperm_UO.relperm(seff);

  return density*relperm;
}
