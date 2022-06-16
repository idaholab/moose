//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDiffusionInterface.h"
#include "FVInterpolationUtils.h"

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
    _coeff1(getFunctor<ADReal>("coeff1")),
    _coeff2(getFunctor<ADReal>("coeff2"))
{
}

ADReal
FVDiffusionInterface::computeQpResidual()
{
  const auto & coef_elem = elemIsOne() ? _coeff1 : _coeff2;
  const auto & coef_neighbor = elemIsOne() ? _coeff2 : _coeff1;

  // Form a finite difference gradient across the interface
  Point one_over_gradient_support = _face_info->elemCentroid() - _face_info->neighborCentroid();
  one_over_gradient_support /= (one_over_gradient_support * one_over_gradient_support);

  const auto gradient = elemIsOne() ? (var1().getElemValue(&_face_info->elem()) -
                                       var2().getElemValue(_face_info->neighborPtr())) *
                                          one_over_gradient_support
                                    : (var1().getElemValue(_face_info->neighborPtr()) -
                                       var2().getElemValue(&_face_info->elem())) *
                                          -one_over_gradient_support;

  ADReal diffusivity = Moose::FV::linearInterpolation(
      coef_elem(elemFromFace()), coef_neighbor(neighborFromFace()), *_face_info, true);

  return -diffusivity * _normal * gradient;
}
