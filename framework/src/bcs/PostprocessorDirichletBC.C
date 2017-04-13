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

#include "PostprocessorDirichletBC.h"

template <>
InputParameters
validParams<PostprocessorDirichletBC>()
{
  InputParameters p = validParams<NodalBC>();
  p.addRequiredParam<PostprocessorName>("postprocessor",
                                        "The postprocessor to set the value to on the boundary.");
  return p;
}

PostprocessorDirichletBC::PostprocessorDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters), _postprocessor_value(getPostprocessorValue("postprocessor"))
{
}

Real
PostprocessorDirichletBC::computeQpResidual()
{
  return _u[_qp] - _postprocessor_value;
}
