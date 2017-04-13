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

#include "PostprocessorAux.h"

template <>
InputParameters
validParams<PostprocessorAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<PostprocessorName>("pp", "The Postprocessor to use as the value");
  return params;
}

PostprocessorAux::PostprocessorAux(const InputParameters & parameters)
  : AuxKernel(parameters), _pp_val(getPostprocessorValue("pp"))
{
}

PostprocessorAux::~PostprocessorAux() {}

Real
PostprocessorAux::computeValue()
{
  return _pp_val;
}
