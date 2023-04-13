//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVOneVarDiffusionInterface.h"
#include "MathFVUtils.h"

registerMooseObject("MooseApp", FVOneVarDiffusionInterface);

InputParameters
FVOneVarDiffusionInterface::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addClassDescription(
      "Computes residual for diffusion operator across an interface for finite volume method.");
  params.addRequiredParam<MaterialPropertyName>("coeff1",
                                                "The diffusion coefficient on the 1st subdomains");
  params.addRequiredParam<MaterialPropertyName>("coeff2",
                                                "The diffusion coefficient on the 2nd subdomains");
  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      coeff_interp_method,
      "Switch that can select face interpolation method for diffusion coefficients.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

FVOneVarDiffusionInterface::FVOneVarDiffusionInterface(const InputParameters & params)
  : FVInterfaceKernel(params),
    _coeff1(getFunctor<ADReal>("coeff1")),
    _coeff2(getFunctor<ADReal>("coeff2"))
{
  if (&var1() != &var2())
    paramError("variable2",
               name(),
               " is only designed to work with the same variable on both sides of an interface.");
  const auto & interp_method = getParam<MooseEnum>("coeff_interp_method");
  if (interp_method == "average")
    _coeff_interp_method = Moose::FV::InterpMethod::Average;
  else if (interp_method == "harmonic")
    _coeff_interp_method = Moose::FV::InterpMethod::HarmonicAverage;
}

ADReal
FVOneVarDiffusionInterface::computeQpResidual()
{
  const auto & coef_elem = elemIsOne() ? _coeff1 : _coeff2;
  const auto & coef_neighbor = elemIsOne() ? _coeff2 : _coeff1;

  const auto & grad = var1().adGradSln(*_face_info, determineState());

  ADReal coef;
  interpolate(_coeff_interp_method,
              coef,
              coef_elem(elemArg(), determineState()),
              coef_neighbor(neighborArg(), determineState()),
              *_face_info,
              true);

  return _normal * -coef * grad;
}
