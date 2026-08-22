//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorCurlPenaltyDirichletBC.h"

registerKokkosResidualObject("MooseTestApp", KokkosVectorCurlPenaltyDirichletBC);

InputParameters
KokkosVectorCurlPenaltyDirichletBC::validParams()
{
  InputParameters params = Moose::Kokkos::VectorIntegratedBC::validParams();
  params.addRequiredParam<Real>("penalty", "The penalty coefficient");
  params.addRequiredParam<FunctionName>("function_x", "The function for the x component");
  params.addParam<FunctionName>("function_y", "The function for the y component");
  params.addParam<FunctionName>("function_z", "The function for the z component");
  params.addClassDescription("Enforces vector tangential Dirichlet data weakly with a penalty.");
  return params;
}

KokkosVectorCurlPenaltyDirichletBC::KokkosVectorCurlPenaltyDirichletBC(
    const InputParameters & parameters)
  : Moose::Kokkos::VectorIntegratedBC(parameters),
    _penalty(getParam<Real>("penalty")),
    _has_function_y(isParamValid("function_y")),
    _has_function_z(isParamValid("function_z")),
    _function_x(getKokkosFunction<KokkosParsedFunction>("function_x")),
    _function_y(
        getKokkosFunction<KokkosParsedFunction>(_has_function_y ? "function_y" : "function_x")),
    _function_z(
        getKokkosFunction<KokkosParsedFunction>(_has_function_z ? "function_z" : "function_x"))
{
}
