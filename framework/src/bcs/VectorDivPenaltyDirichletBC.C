//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorDivPenaltyDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", VectorDivPenaltyDirichletBC);

InputParameters
VectorDivPenaltyDirichletBC::validParams()
{
  InputParameters params = VectorIntegratedBC::validParams();
  params.addRequiredParam<Real>("penalty", "The penalty coefficient");
  params.addParam<FunctionName>("function",
                                "The boundary condition vector function, "
                                "use as an alternative to a component-wise specification");
  params.addParam<FunctionName>("function_x", 0, "The function for the x component");
  params.addParam<FunctionName>("function_y", 0, "The function for the y component");
  params.addParam<FunctionName>("function_z", 0, "The function for the z component");
  params.addClassDescription("Enforces, in a weak sense, a Dirichlet boundary condition on the "
                             "divergence of a nonlinear vector variable by applying a penalty to "
                             "the difference between the current solution and the Dirichlet data.");
  return params;
}

VectorDivPenaltyDirichletBC::VectorDivPenaltyDirichletBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),
    _penalty(getParam<Real>("penalty")),
    _function(isParamValid("function") ? &getFunction("function") : nullptr),
    _function_x(getFunction("function_x")),
    _function_y(getFunction("function_y")),
    _function_z(getFunction("function_z"))
{
}

Real
VectorDivPenaltyDirichletBC::computeQpResidual()
{
  RealVectorValue u_exact;
  if (_function)
    u_exact = _function->vectorValue(_t, _q_point[_qp]);
  else
    u_exact = {_function_x.value(_t, _q_point[_qp]),
               _function_y.value(_t, _q_point[_qp]),
               _function_z.value(_t, _q_point[_qp])};

  return _penalty * ((_u[_qp] - u_exact) * _normals[_qp]) * (_test[_i][_qp] * _normals[_qp]);
}

Real
VectorDivPenaltyDirichletBC::computeQpJacobian()
{
  return _penalty * (_phi[_j][_qp] * _normals[_qp]) * (_test[_i][_qp] * _normals[_qp]);
}
