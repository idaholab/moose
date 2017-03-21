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

#include "PostprocessorNeumannBC.h"

template <>
InputParameters
validParams<PostprocessorNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<PostprocessorName>(
      "postprocessor", 0.0, "The postprocessor to use for value of the gradient on the boundary.");
  return params;
}

PostprocessorNeumannBC::PostprocessorNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _value(getPostprocessorValue("postprocessor"))
{
}

Real
PostprocessorNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _value;
}
