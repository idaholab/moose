//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDiffusionFunctionBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FVDiffusionFunctionBC);

InputParameters
FVDiffusionFunctionBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Imposes the integrated boundary condition "
                             "arising from integration by parts of a Laplacian operator, and "
                             "where the exact solution can be specified.");
  params.addRequiredParam<FunctionName>("exact_solution", "The exact solution.");
  params.addRequiredParam<MaterialPropertyName>("coeff", "diffusion coefficient");
  params.addRequiredParam<FunctionName>("coeff_function", "A function describing the coefficient");
  return params;
}

FVDiffusionFunctionBC::FVDiffusionFunctionBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _exact_solution(getFunction("exact_solution")),
    _coeff(getADMaterialProperty<Real>("coeff")),
    _coeff_function(getFunction("coeff_function"))
{
}

ADReal
FVDiffusionFunctionBC::computeQpResidual()
{
  auto u_ghost =
      _exact_solution.value(_t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid());

  auto dudn = Moose::FV::gradUDotNormal(_u[_qp], u_ghost, *_face_info, _var);

  auto coeff_ghost =
      _coeff_function.value(_t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid());

  ADReal k;
  interpolate(Moose::FV::InterpMethod::Average, k, _coeff[_qp], coeff_ghost, *_face_info, true);

  return -1 * k * dudn;
}
