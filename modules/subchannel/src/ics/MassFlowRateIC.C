#include "MassFlowRateIC.h"

registerMooseObject("SubChannelApp", MassFlowRateIC);

InputParameters
MassFlowRateIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addParam<Real>("mass_flux", "Specified mass flux [kg/s-m^2]");
  params.addClassDescription("Computes mass float rate from specified mass flux and cross-sectional area");
  return params;
}

MassFlowRateIC::MassFlowRateIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mass_flux(getParam<Real>("mass_flux")),
    _area(coupledValue("area"))
{
}

Real
MassFlowRateIC::value(const Point & /*p*/)
{
  return _mass_flux * _area[_qp];
}
