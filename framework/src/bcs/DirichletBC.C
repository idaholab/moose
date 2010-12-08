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
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.set<bool>("_integrated") = false;
  return params;
}

DirichletBC::DirichletBC(const std::string & name, InputParameters parameters)
  :BoundaryCondition(name, parameters),
    _value(getParam<Real>("value"))
{}

Real
DirichletBC::computeQpResidual()
{
  return _u[_qp]-_value;
}
