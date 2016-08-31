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

#include "ArrayDirichletBC.h"

template<>
InputParameters validParams<ArrayDirichletBC>()
{
  InputParameters p = validParams<ArrayNodalBC>();
  p.addRequiredParam<Real>("value", "Value of the BC");
  return p;
}


ArrayDirichletBC::ArrayDirichletBC(const InputParameters & parameters) :
  ArrayNodalBC(parameters),
  _value(_array_var.count())
{
  _value.setConstant(getParam<Real>("value"));
}

void
ArrayDirichletBC::computeQpResidual()
{
  _residual.noalias() = _u[_qp] - _value;
}
