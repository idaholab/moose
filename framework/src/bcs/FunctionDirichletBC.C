/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FunctionDirichletBC.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<bool>("_integrated") = false;
  params.addRequiredParam<std::string>("function", "The forcing function.");
  return params;
}

FunctionDirichletBC::FunctionDirichletBC(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
  :BoundaryCondition(name, sys, parameters),
  _func(getFunction("function"))
{
}

Real
FunctionDirichletBC::f()
{
  return _func(_t, (*_current_node)(0), (*_current_node)(1), (*_current_node)(2));
}

Real
FunctionDirichletBC::computeQpResidual()
{
  return _u[_qp]-f();
}
