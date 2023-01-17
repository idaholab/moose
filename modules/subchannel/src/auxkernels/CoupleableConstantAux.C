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

#include "CoupleableConstantAux.h"
registerMooseObject("MooseApp", CoupleableConstantAux);

InputParameters
CoupleableConstantAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Creates a constant field in the domain. Able to read value from postprocessor");
  params.addParam<Real>("value", 1.0, "Some constant value that can be read from the input file");
  params.addParam<PostprocessorName>(
      "postprocessor", 1.0, "The postprocessor to use for the value");
  params.declareControllable("value");
  return params;
}

CoupleableConstantAux::CoupleableConstantAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _value(getParam<Real>("value")),
    _pvalue(getPostprocessorValue("postprocessor"))
{
  if (parameters.isParamSetByUser("value") && parameters.isParamSetByUser("postprocessor"))
    mooseError(name(), ": Please provide only one user defined value");
}

Real
CoupleableConstantAux::computeValue()
{
  return _value * _pvalue;
}
