/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "FunctionValuePostprocessor.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionValuePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<FunctionName>("function",
                                        "The function which supplies the postprocessor value.");
  params.addParam<Point>(
      "point", Point(), "A point in space to be given to the function Default: (0, 0, 0)");
  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the function");

  params.declareControllable("point scale_factor");

  return params;
}

FunctionValuePostprocessor::FunctionValuePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _function(getFunction("function")),
    _point(getParam<Point>("point")),
    _scale_factor(getParam<Real>("scale_factor"))
{
}

void
FunctionValuePostprocessor::initialize()
{
}

void
FunctionValuePostprocessor::execute()
{
}

PostprocessorValue
FunctionValuePostprocessor::getValue()
{
  return _scale_factor * _function.value(_t, _point);
}
