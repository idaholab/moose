//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCurlPenaltyDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", VectorCurlPenaltyDirichletBC);

InputParameters
VectorCurlPenaltyDirichletBC::validParams()
{
  InputParameters params = VectorIntegratedBC::validParams();
  params.addRequiredParam<Real>("penalty", "The penalty coefficient");
  params.addParam<FunctionName>("function_x", 0, "The function for the x component");
  params.addParam<FunctionName>("function_y", 0, "The function for the y component");
  params.addParam<FunctionName>("function_z", 0, "The function for the z component");

  params.addDeprecatedParam<FunctionName>(
      "x_exact_soln", "The exact solution for the x component", "Use 'function_x' instead.");
  params.addDeprecatedParam<FunctionName>(
      "y_exact_soln", "The exact solution for the y component", "Use 'function_y' instead.");
  params.addDeprecatedParam<FunctionName>(
      "z_exact_soln", "The exact solution for the z component", "Use 'function_z' instead.");
  return params;
}

VectorCurlPenaltyDirichletBC::VectorCurlPenaltyDirichletBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),
    _penalty(getParam<Real>("penalty")),
    _function(isParamValid("function") ? &getFunction("function") : nullptr),
    _function_x(isParamValid("x_exact_soln") ? getFunction("x_exact_soln")
                                             : getFunction("function_x")),
    _function_y(isParamValid("y_exact_soln") ? getFunction("y_exact_soln")
                                             : getFunction("function_y")),
    _function_z(isParamValid("z_exact_soln") ? getFunction("z_exact_soln")
                                             : getFunction("function_z"))
{
  if (_function &&
      (parameters.isParamSetByUser("function_x") || parameters.isParamSetByUser("x_exact_soln")))
    paramError("function_x", "The 'function' and 'function_x' parameters cannot both be set.");
  if (_function &&
      (parameters.isParamSetByUser("function_y") || parameters.isParamSetByUser("y_exact_soln")))
    paramError("function_y", "The 'function' and 'function_y' parameters cannot both be set.");
  if (_function &&
      (parameters.isParamSetByUser("function_z") || parameters.isParamSetByUser("z_exact_soln")))
    paramError("function_z", "The 'function' and 'function_z' parameters cannot both be set.");
}

Real
VectorCurlPenaltyDirichletBC::computeQpResidual()
{
  RealVectorValue u_exact;
  if (_function)
    u_exact = _function->vectorValue(_t, _q_point[_qp]);
  else
    u_exact = {_function_x.value(_t, _q_point[_qp]),
               _function_y.value(_t, _q_point[_qp]),
               _function_z.value(_t, _q_point[_qp])};
  RealVectorValue Ncu = (_u[_qp] - u_exact).cross(_normals[_qp]);
  return _penalty * Ncu * ((_test[_i][_qp]).cross(_normals[_qp]));
}

Real
VectorCurlPenaltyDirichletBC::computeQpJacobian()
{
  return _penalty * (_phi[_j][_qp]).cross(_normals[_qp]) * (_test[_i][_qp]).cross(_normals[_qp]);
}
