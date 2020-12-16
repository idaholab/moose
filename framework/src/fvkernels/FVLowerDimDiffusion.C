//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVLowerDimDiffusion.h"

registerMooseObject("MooseApp", FVLowerDimDiffusion);

InputParameters
FVLowerDimDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes residual for diffusion operator for finite volume method.");
  params.addRequiredParam<MaterialPropertyName>("coeff", "diffusion coefficient");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<Real>("coeff_LD", 1.0, "diffusion coefficient transversal to the LowerDim");
  params.addParam<Real>("thickness", 1.0, "thickness of the flat LowerDim");
  return params;
}

FVLowerDimDiffusion::FVLowerDimDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
    _coeff_elem(getADMaterialProperty<Real>("coeff")),
    _coeff_neighbor(getNeighborADMaterialProperty<Real>("coeff")),
    _coeff_LD(getParam<Real>("coeff_LD")),
    _thickness(getParam<Real>("thickness"))
{
  if (_thickness<=0)
    mooseError("thickness has to be strictly positive");
}

ADReal
FVLowerDimDiffusion::computeQpResidual()
{
  auto dudn = gradUDotNormal();

  auto dist_elem = (_face_info->faceCentroid() - _face_info->elemCentroid()).norm();
  auto dist_neighbor = (_face_info->faceCentroid() - _face_info->neighborCentroid()).norm();
  auto coeff_neighbor = _coeff_neighbor[_qp];
  if (!_face_info->isBoundary() // to be able to call neighbor ptr
      && _face_info->elem().dim()>_face_info->neighbor().dim()) // then we are dealing with the transverse flow to the LowerDim (always oriented towards the LowerDim)
  {
    coeff_neighbor = _coeff_LD;
    dist_neighbor = _thickness/2.;
    dist_elem = (_face_info->faceCentroid() - _face_info->elemCentroid()).norm(); // - _thickness/2.;
  }

  ADReal k = (dist_neighbor * _coeff_elem[_qp] + dist_elem * coeff_neighbor) / (dist_elem + dist_neighbor); //average weighted with distance
  
  return -1 * k * dudn;
}
