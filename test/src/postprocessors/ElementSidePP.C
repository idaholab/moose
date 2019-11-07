//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSidePP.h"

registerMooseObject("MooseTestApp", ElementSidePP);

InputParameters
ElementSidePP::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredParam<PostprocessorName>("side_pp", "Side postprocessor to be passed in");
  return params;
}

ElementSidePP::ElementSidePP(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters), _sidepp(getPostprocessorValue("side_pp"))
{
}

Real
ElementSidePP::getValue()
{
  return _sidepp;
}

Real
ElementSidePP::computeQpIntegral()
{
  // dummy to make the interface happy
  return 0;
}
