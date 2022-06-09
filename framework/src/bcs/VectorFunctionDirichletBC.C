//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorFunctionDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", VectorFunctionDirichletBC);
registerMooseObjectRenamed("MooseApp",
                           LagrangeVecFunctionDirichletBC,
                           "05/01/2019 00:01",
                           VectorFunctionDirichletBC);

InputParameters
VectorFunctionDirichletBC::validParams()
{
  InputParameters params = VectorNodalBC::validParams();
  params.addClassDescription(
      "Imposes the essential boundary condition $\\vec{u}=\\vec{g}$, where $\\vec{g}$ "
      "components are calculated with functions.");

  params.addParam<FunctionName>("function",
                                "The boundary condition vector function. This cannot be supplied "
                                "with the component parameters.");

  params.addParam<FunctionName>("function_x", 0, "The function for the x component");
  params.addParam<FunctionName>("function_y", 0, "The function for the y component");
  params.addParam<FunctionName>("function_z", 0, "The function for the z component");
  return params;
}

VectorFunctionDirichletBC::VectorFunctionDirichletBC(const InputParameters & parameters)
  : VectorNodalBC(parameters),
    _function(isParamValid("function") ? &getFunction("function") : nullptr),
    _function_x(getFunction("function_x")),
    _function_y(getFunction("function_y")),
    _function_z(getFunction("function_z"))
{
}

RealVectorValue
VectorFunctionDirichletBC::computeQpResidual()
{
  if (_function)
    _values = _function->vectorValue(_t, *_current_node);
  else
    _values = {_function_x.value(_t, *_current_node),
               _function_y.value(_t, *_current_node),
               _function_z.value(_t, *_current_node)};

  return _u - _values;
}
