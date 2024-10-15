/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "MassFlowRateIC.h"

registerMooseObject("SubChannelApp", MassFlowRateIC);

InputParameters
MassFlowRateIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Computes mass float rate from specified mass flux and cross-sectional area");
  params.addRequiredCoupledVar("area", "Subchannel surface area [m^2]");
  params.addRequiredParam<Real>("mass_flux", "Specified mass flux [kg/s-m^2]");
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
