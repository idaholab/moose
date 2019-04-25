//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADLagrangeVecFunctionDirichletBC.h"
#include "Function.h"

registerADMooseObject("MooseApp", ADLagrangeVecFunctionDirichletBC);

defineADValidParams(
    ADLagrangeVecFunctionDirichletBC,
    ADVectorNodalBC,
    params.addClassDescription(
        "Imposes the essential boundary condition $\\vec{u}=\\vec{g}$, where $\\vec{g}$ "
        "components are calculated with functions.");
    params.addParam<FunctionName>("x_exact_soln", 0, "The exact solution for the x component");
    params.addParam<FunctionName>("y_exact_soln", 0, "The exact solution for the y component");
    params.addParam<FunctionName>("z_exact_soln", 0, "The exact solution for the z component"););

template <ComputeStage compute_stage>
ADLagrangeVecFunctionDirichletBC<compute_stage>::ADLagrangeVecFunctionDirichletBC(
    const InputParameters & parameters)
  : ADVectorNodalBC<compute_stage>(parameters),
    _exact_x(getFunction("x_exact_soln")),
    _exact_y(getFunction("y_exact_soln")),
    _exact_z(getFunction("z_exact_soln"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
ADLagrangeVecFunctionDirichletBC<compute_stage>::computeQpResidual()
{
  _values = RealVectorValue(_exact_x.value(_t, *_current_node),
                            _exact_y.value(_t, *_current_node),
                            _exact_z.value(_t, *_current_node));
  return _u - _values;
}
