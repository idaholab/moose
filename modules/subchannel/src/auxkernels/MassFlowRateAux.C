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

#include "MassFlowRateAux.h"

registerMooseObject("SubChannelApp", MassFlowRateAux);

InputParameters
MassFlowRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes mass flow rate from specified mass flux and subchannel cross-sectional "
      "area. Reads postprocessor value");
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addRequiredParam<PostprocessorName>(
      "mass_flux", "The postprocessor or Real to use for the value of mass_flux");
  return params;
}

MassFlowRateAux::MassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _mass_flux(getPostprocessorValue("mass_flux")),
    _area(coupledValue("area"))
{
}

Real
MassFlowRateAux::computeValue()
{
  return _mass_flux * _area[_qp];
}
