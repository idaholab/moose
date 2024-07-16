//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kOmegaSSTSigmaKAux.h"

registerMooseObject("NavierStokesApp", kOmegaSSTSigmaKAux);

InputParameters
kOmegaSSTSigmaKAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the scaling factor for diffusion of TKE.");
  params.addRequiredParam<MooseFunctorName>("F1", "The F1 blending function.");
  return params;
}

kOmegaSSTSigmaKAux::kOmegaSSTSigmaKAux(const InputParameters & parameters)
  : AuxKernel(parameters), _F1(getFunctor<ADReal>("F1"))
{
  if (!dynamic_cast<MooseVariableFV<Real> *>(&_var))
    paramError("variable",
               "'",
               name(),
               "' is currently programmed to use finite volume machinery, so make sure that '",
               _var.name(),
               "' is a finite volume variable.");
}

Real
kOmegaSSTSigmaKAux::computeValue()
{
  const auto F1 = _F1(makeElemArg(_current_elem), determineState());
  const auto blend_coef = 1.0 / (F1 * _sigma_k_1 + (1.0 - F1) * _sigma_k_2);
  return blend_coef.value();
}
