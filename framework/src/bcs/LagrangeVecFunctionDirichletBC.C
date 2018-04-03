//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LagrangeVecFunctionDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", LagrangeVecFunctionDirichletBC);

template <>
InputParameters
validParams<LagrangeVecFunctionDirichletBC>()
{
  InputParameters p = validParams<VectorNodalBC>();
  p.addClassDescription(
      "Imposes the essential boundary condition $\\vec{u}=\\vec{g}$, where $\\vec{g}$ "
      "components are calculated with functions.");
  p.addParam<FunctionName>("x_exact_soln", 0, "The exact solution for the x component");
  p.addParam<FunctionName>("y_exact_soln", 0, "The exact solution for the y component");
  p.addParam<FunctionName>("z_exact_soln", 0, "The exact solution for the z component");
  return p;
}

LagrangeVecFunctionDirichletBC::LagrangeVecFunctionDirichletBC(const InputParameters & parameters)
  : VectorNodalBC(parameters),
    _exact_x(getFunction("x_exact_soln")),
    _exact_y(getFunction("y_exact_soln")),
    _exact_z(getFunction("z_exact_soln"))
{
}

RealVectorValue
LagrangeVecFunctionDirichletBC::computeQpResidual()
{
  _values = {_exact_x.value(_t, *_current_node),
             _exact_y.value(_t, *_current_node),
             _exact_z.value(_t, *_current_node)};

  return _u - _values;
}
