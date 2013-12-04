//  This post processor returns the effective saturation of a region.
//
#include "RichardsSeffAux.h"

template<>
InputParameters validParams<RichardsSeffAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_var", "The variable that represents the pressure");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<Real>("p_air", "Air pressure.  Typical value=101E3");
  params.addClassDescription("auxillary variable which is effective saturation");
  return params;
}

RichardsSeffAux::RichardsSeffAux(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _pressure_var(coupledValue("pressure_var")),
  _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
  _p_air(getParam<Real>("p_air"))
{}

Real
RichardsSeffAux::computeValue()
{
  Real pc = _p_air - _pressure_var[_qp];
  Real seff = _seff_UO.seff(pc);
  return seff;
}
