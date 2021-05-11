//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDiffusionInterface.h"
#include "FVUtils.h"

registerMooseObject("MooseApp", FVDiffusionInterface);

InputParameters
FVDiffusionInterface::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addClassDescription("Computes the residual for diffusion operator across an interface for "
                             "the finite volume method.");
  params.addRequiredParam<MaterialPropertyName>("coeff1",
                                                "The diffusion coefficient on the 1st subdomain");
  params.addRequiredParam<MaterialPropertyName>("coeff2",
                                                "The diffusion coefficient on the 2nd subdomain");
  return params;
}

FVDiffusionInterface::FVDiffusionInterface(const InputParameters & params)
  : FVInterfaceKernel(params),
    _coeff1_elem(getADMaterialProperty<Real>("coeff1")),
    _coeff2_elem(getADMaterialProperty<Real>("coeff2")),
    _coeff1_neighbor(getNeighborADMaterialProperty<Real>("coeff1")),
    _coeff2_neighbor(getNeighborADMaterialProperty<Real>("coeff2"))
{
}

ADReal
FVDiffusionInterface::computeQpResidual()
{
  const auto & coef_elem = elemIsOne() ? _coeff1_elem : _coeff2_elem;
  const auto & coef_neighbor = elemIsOne() ? _coeff2_neighbor : _coeff1_neighbor;

  // Form a finite difference gradient across the interface
  Point one_over_gradient_support = _face_info->elemCentroid() - _face_info->neighborCentroid();
  one_over_gradient_support /= (one_over_gradient_support * one_over_gradient_support);
  const auto gradient = elemIsOne() ? (var1().getElemValue(&_face_info->elem()) -
                                       var2().getElemValue(_face_info->neighborPtr())) *
                                          one_over_gradient_support
                                    : (var1().getElemValue(_face_info->neighborPtr()) -
                                       var2().getElemValue(&_face_info->elem())) *
                                          -one_over_gradient_support;

  ADReal diffusivity;
  interpolate(Moose::FV::InterpMethod::Average,
              diffusivity,
              coef_elem[_qp],
              coef_neighbor[_qp],
              *_face_info,
              true);

  return -diffusivity * _normal * gradient;
}
