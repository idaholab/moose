//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorFunctionDirichletBC.h"

registerKokkosResidualObject("MooseApp", KokkosVectorFunctionDirichletBC);

InputParameters
KokkosVectorFunctionDirichletBC::validParams()
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

KokkosVectorFunctionDirichletBC::KokkosVectorFunctionDirichletBC(const InputParameters & parameters)
  : VectorNodalBC(parameters),
    _function(isParamValid("function") ? getKokkosFunction("function") : Moose::Kokkos::Function()),
    _function_x(getKokkosFunction("function_x")),
    _function_y(getKokkosFunction("function_y")),
    _function_z(getKokkosFunction("function_z")),
    _has_function(isParamValid("function"))
{
  if (_has_function && parameters.isParamSetByUser("function_x"))
    paramError("function_x", "The 'function' and 'function_x' parameters cannot both be set.");
  if (_has_function && parameters.isParamSetByUser("function_y"))
    paramError("function_y", "The 'function' and 'function_y' parameters cannot both be set.");
  if (_has_function && parameters.isParamSetByUser("function_z"))
    paramError("function_z", "The 'function' and 'function_z' parameters cannot both be set.");
}
