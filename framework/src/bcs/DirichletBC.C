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

template <>
InputParameters
validParams<DirichletBC>()
{
  InputParameters p = validParams<NodalBC>();
  p.addRequiredParam<Real>("value", "Value of the BC");
  p.declareControllable("value");
  p.addClassDescription("Imposes the essential boundary condition $u=g$, where $g$ "
                        "is a constant, controllable value.");
  return p;
}

DirichletBC::DirichletBC(const InputParameters & parameters)
  : NodalBC(parameters), _value(getParam<Real>("value"))
{
}

Real
DirichletBC::computeQpResidual()
{
  return _u[_qp] - _value;
}
