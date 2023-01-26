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

#include "PostprocessorMassFlowRateAux.h"

registerMooseObject("SubChannelApp", PostprocessorMassFlowRateAux);

InputParameters
PostprocessorMassFlowRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes mass flow rate from specified mass flux and cross-sectional "
                             "area. Reads postprocessor value");
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addRequiredParam<PostprocessorName>("postprocessor",
                                             "The postprocessor to use for the value of mass_flux");
  return params;
}

PostprocessorMassFlowRateAux::PostprocessorMassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pvalue(getPostprocessorValue("postprocessor")),
    _area(coupledValue("area"))
{
}

Real
PostprocessorMassFlowRateAux::computeValue()
{
  return _pvalue * _area[_qp];
}
