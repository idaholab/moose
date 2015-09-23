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

#include "DirichletBC.h"

template<>
InputParameters validParams<DirichletBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredParam<PostprocessorName>("value", "Value of the BC");
  return params;
}


DirichletBC::DirichletBC(const InputParameters & parameters) :
  NodalBC(parameters),
  _value(getPostprocessorValue("value"))
{}

Real
DirichletBC::computeQpResidual()
{
  return _u[_qp] - _value;
}
