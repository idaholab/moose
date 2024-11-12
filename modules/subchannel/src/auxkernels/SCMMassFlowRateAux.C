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

#include "SCMMassFlowRateAux.h"

registerMooseObject("SubChannelApp", SCMMassFlowRateAux);

InputParameters
SCMMassFlowRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes mass flow rate from specified mass flux and subchannel cross-sectional "
      "area. Can read either PostprocessorValue or Real");
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addRequiredParam<PostprocessorName>(
      "mass_flux", "The postprocessor or Real to use for the value of mass_flux");
  return params;
}

SCMMassFlowRateAux::SCMMassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _mass_flux(getPostprocessorValue("mass_flux")),
    _area(coupledValue("area"))
{
}

Real
SCMMassFlowRateAux::computeValue()
{
  return _mass_flux * _area[_qp];
}
