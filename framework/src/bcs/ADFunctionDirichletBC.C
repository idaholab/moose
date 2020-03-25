//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFunctionDirichletBC.h"
#include "Function.h"

#include "libmesh/node.h"

registerMooseObject("MooseApp", ADFunctionDirichletBC);

InputParameters
ADFunctionDirichletBC::validParams()
{
  InputParameters params = ADDirichletBCBase::validParams();
  params.addClassDescription("Imposes the essential boundary condition $u=g$, where $g$ "
                             "is calculated by a function.");
  params.addParam<FunctionName>("function", 0, "The function describing the Dirichlet condition");
  return params;
}

ADFunctionDirichletBC::ADFunctionDirichletBC(const InputParameters & parameters)
  : ADDirichletBCBase(parameters), _function(getFunction("function"))
{
}

ADReal
ADFunctionDirichletBC::computeQpValue()
{
  return _function.value(_t, *_current_node);
}
