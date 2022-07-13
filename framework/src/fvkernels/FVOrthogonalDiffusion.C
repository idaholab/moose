//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVOrthogonalDiffusion.h"

registerMooseObject("MooseApp", FVOrthogonalDiffusion);

InputParameters
FVOrthogonalDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Imposes an orthogonal diffusion term.");
  params.addRequiredParam<MaterialPropertyName>("coeff", "diffusion coefficient");
  params.addParam<MaterialPropertyName>(
      "diffusing_quantity",
      "The quantity that is diffusing. By default, the 'variable' solution value will be used.");
  return params;
}

FVOrthogonalDiffusion::FVOrthogonalDiffusion(const InputParameters & parameters)
  : FVFluxKernel(parameters),
    _coeff_elem(getADMaterialProperty<Real>("coeff")),
    _coeff_neighbor(getNeighborADMaterialProperty<Real>("coeff")),
    _diff_quant_elem(isParamValid("diffusing_quantity")
                         ? getADMaterialProperty<Real>("diffusing_quantity").get()
                         : _u_elem),
    _diff_quant_neighbor(isParamValid("diffusing_quantity")
                             ? getNeighborADMaterialProperty<Real>("diffusing_quantity").get()
                             : _u_neighbor)
{
}

ADReal
FVOrthogonalDiffusion::computeQpResidual()
{
  ADReal coeff_interface;
  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                         coeff_interface,
                         _coeff_elem[_qp],
                         _coeff_neighbor[_qp],
                         *_face_info,
                         true);

  return -coeff_interface * (_diff_quant_neighbor[_qp] - _diff_quant_elem[_qp]) /
         _face_info->dCNMag();
}
