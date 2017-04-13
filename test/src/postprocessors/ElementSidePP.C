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

#include "ElementSidePP.h"

template <>
InputParameters
validParams<ElementSidePP>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
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
