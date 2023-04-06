//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDiffusionInterface.h"
#include "MathFVUtils.h"

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
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      coeff_interp_method,
      "Switch that can select face interpolation method for diffusion coefficients.");
  return params;
}

FVDiffusionInterface::FVDiffusionInterface(const InputParameters & params)
  : FVInterfaceKernel(params),
    _coeff1(getFunctor<ADReal>("coeff1")),
    _coeff2(getFunctor<ADReal>("coeff2"))
{
  const auto & interp_method = getParam<MooseEnum>("coeff_interp_method");
  if (interp_method == "average")
    _coeff_interp_method = Moose::FV::InterpMethod::Average;
  else if (interp_method == "harmonic")
    _coeff_interp_method = Moose::FV::InterpMethod::HarmonicAverage;
}

ADReal
FVDiffusionInterface::computeQpResidual()
{
  const auto & coef_elem = elemIsOne() ? _coeff1 : _coeff2;
  const auto & coef_neighbor = elemIsOne() ? _coeff2 : _coeff1;

  // Form a finite difference gradient across the interface
  Point one_over_gradient_support = _face_info->elemCentroid() - _face_info->neighborCentroid();
  one_over_gradient_support /= (one_over_gradient_support * one_over_gradient_support);
  const auto state = determineState();
  const auto gradient = elemIsOne() ? (var1().getElemValue(&_face_info->elem(), state) -
                                       var2().getElemValue(_face_info->neighborPtr(), state)) *
                                          one_over_gradient_support
                                    : (var1().getElemValue(_face_info->neighborPtr(), state) -
                                       var2().getElemValue(&_face_info->elem(), state)) *
                                          -one_over_gradient_support;

  ADReal diffusivity;
  interpolate(_coeff_interp_method,
              diffusivity,
              coef_elem(elemArg(), determineState()),
              coef_neighbor(neighborArg(), determineState()),
              *_face_info,
              true);

  return -diffusivity * _normal * gradient;
}
