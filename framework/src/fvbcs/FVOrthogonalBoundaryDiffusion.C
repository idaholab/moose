//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVOrthogonalBoundaryDiffusion.h"
#include "Function.h"

registerMooseObject("MooseApp", FVOrthogonalBoundaryDiffusion);

InputParameters
FVOrthogonalBoundaryDiffusion::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Imposes an orthogonal diffusion boundary term with specified boundary function.");
  params.addRequiredParam<FunctionName>("function",
                                        "The value of the quantity of interest on the boundary.");
  params.addRequiredParam<MaterialPropertyName>("coeff", "diffusion coefficient");
  params.addParam<MaterialPropertyName>(
      "diffusing_quantity",
      "The quantity that is diffusing. By default, the 'variable' solution value will be used.");
  return params;
}

FVOrthogonalBoundaryDiffusion::FVOrthogonalBoundaryDiffusion(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _function(getFunction("function")),
    _coeff_elem(getADMaterialProperty<Real>("coeff")),
    _coeff_neighbor(getNeighborADMaterialProperty<Real>("coeff")),
    _diff_quant_elem(isParamValid("diffusing_quantity")
                         ? getADMaterialProperty<Real>("diffusing_quantity").get()
                         : _u),
    _diff_quant_neighbor(isParamValid("diffusing_quantity")
                             ? getNeighborADMaterialProperty<Real>("diffusing_quantity").get()
                             : _u_neighbor)
{
}

ADReal
FVOrthogonalBoundaryDiffusion::computeQpResidual()
{
  const bool elem_is_interior =
      _face_info->faceType(_var.name()) == FaceInfo::VarFaceNeighbors::ELEM;

  const auto & diff_quant = elem_is_interior ? _diff_quant_elem[_qp] : _diff_quant_neighbor[_qp];
  const auto & coeff = elem_is_interior ? _coeff_elem[_qp] : _coeff_neighbor[_qp];
  const auto & interior_centroid =
      elem_is_interior ? _face_info->elemCentroid() : _face_info->neighborCentroid();

  return -coeff * (_function.value(_t, _face_info->faceCentroid()) - diff_quant) /
         (_face_info->faceCentroid() - interior_centroid).norm();
}
