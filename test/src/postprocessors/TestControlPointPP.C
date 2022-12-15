//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestControlPointPP.h"
#include "Function.h"

registerMooseObject("MooseTestApp", TestControlPointPP);

InputParameters
TestControlPointPP::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<FunctionName>("function",
                                        "The function which supplies the postprocessor value.");
  params.addParam<Point>(
      "point", Point(), "A point in space to be given to the function Default: (0, 0, 0)");
  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the function");

  params.declareControllable("point scale_factor");
  params.addClassDescription("Tests a controllable parameter of type Point");
  return params;
}

TestControlPointPP::TestControlPointPP(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _function(getFunction("function")),
    _point(getParam<Point>("point")),
    _scale_factor(getParam<Real>("scale_factor"))
{
}

void
TestControlPointPP::initialize()
{
}

void
TestControlPointPP::execute()
{
}

PostprocessorValue
TestControlPointPP::getValue()
{
  return _scale_factor * _function.value(_t, _point);
}
