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
      "Computes mass flow rate from specified mass flux and cross-sectional area");
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addParam<Real>("mass_flux", 1.0, "User specified mass flux [kg/s-m^2]");
  params.addParam<PostprocessorName>(
      "postprocessor", 1.0, "The postprocessor to use for the value of mass_flux");
  params.declareControllable("mass_flux");
  return params;
}

MassFlowRateAux::MassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _mass_flux(getParam<Real>("mass_flux")),
    _value(getPostprocessorValue("postprocessor")),
    _area(coupledValue("area"))
{
  if (parameters.isParamSetByUser("mass_flux") && parameters.isParamSetByUser("postprocessor"))
    mooseError(name(), ": Please provide only one user defined mass_flux");
}

Real
MassFlowRateAux::computeValue()
{
  return _value * _mass_flux * _area[_qp];
}
