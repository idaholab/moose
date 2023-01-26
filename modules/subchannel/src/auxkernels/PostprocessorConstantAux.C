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

#include "PostprocessorConstantAux.h"
registerMooseObject("MooseApp", PostprocessorConstantAux);

InputParameters
PostprocessorConstantAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Creates a constant field in the domain. Reads value from postprocessor");
  params.addRequiredParam<PostprocessorName>("postprocessor",
                                             "The postprocessor to use for the value");
  return params;
}

PostprocessorConstantAux::PostprocessorConstantAux(const InputParameters & parameters)
  : AuxKernel(parameters), _pvalue(getPostprocessorValue("postprocessor"))
{
}

Real
PostprocessorConstantAux::computeValue()
{
  return _pvalue;
}
