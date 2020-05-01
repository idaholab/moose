//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDiffusion.h"

registerMooseObject("MooseApp", FVDiffusion);

InputParameters
FVDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("coeff", "diffusion coefficient");
  return params;
}

FVDiffusion::FVDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _coeff_elem(getADMaterialProperty<Real>("coeff")),
    _coeff_neighbor(getNeighborADMaterialProperty<Real>("coeff"))
{
}

ADReal
FVDiffusion::computeQpResidual()
{
  auto dudn = gradUDotNormal();

  // Eventually, it will be nice to offer automatic-switching triggered by
  // input parameters to change between different interpolation methods for
  // this.
  ADReal k;
  interpolate(InterpMethod::Average, k, _coeff_elem[_qp], _coeff_neighbor[_qp]);

  return -1 * k * dudn;
}
