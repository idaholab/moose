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

#include "PostprocessorFunction.h"
#include "MooseTypes.h"

template<>
InputParameters validParams<PostprocessorFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<PostprocessorName>("pp", "The name of the postprocessor you are trying to get.");
  return params;
}

PostprocessorFunction::PostprocessorFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _pp(getPostprocessorValue("pp"))
{
}

Real
PostprocessorFunction::value(Real /*t*/, const Point & /*p*/)
{
  return _pp;
}
