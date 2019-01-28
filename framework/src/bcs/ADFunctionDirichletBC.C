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

registerADMooseObject("MooseApp", ADFunctionDirichletBC);

defineADValidParams(
    ADFunctionDirichletBC,
    ADNodalBC,
    params.addClassDescription("Imposes the essential boundary condition $u=g$, where $g$ "
                               "is calculated by a function.");
    params.addParam<FunctionName>("function",
                                  0,
                                  "The function describing the Dirichlet condition"););

template <ComputeStage compute_stage>
ADFunctionDirichletBC<compute_stage>::ADFunctionDirichletBC(const InputParameters & parameters)
  : ADNodalBC<compute_stage>(parameters), _function(getFunction("function"))
{
}

template <ComputeStage compute_stage>
ADReal
ADFunctionDirichletBC<compute_stage>::computeQpResidual()
{
  return _u - _function.value(_t, *_current_node);
}
