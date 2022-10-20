#include "MassFlowRateAux.h"

registerMooseObject("SubChannelApp", MassFlowRateAux);

InputParameters
MassFlowRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes mass float rate from specified mass flux and cross-sectional area");
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addRequiredParam<Real>("mass_flux", "Specified mass flux [kg/s-m^2]");
  params.declareControllable("mass_flux");
  return params;
}

MassFlowRateAux::MassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters), _mass_flux(getParam<Real>("mass_flux")), _area(coupledValue("area"))
{
}

Real
MassFlowRateAux::computeValue()
{
  return _mass_flux * _area[_qp];
}
