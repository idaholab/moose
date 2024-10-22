//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kOmegaSSTSigmaOmegaAux.h"

registerMooseObject("NavierStokesApp", kOmegaSSTSigmaOmegaAux);

InputParameters
kOmegaSSTSigmaOmegaAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the scaling factor for diffusion of TKESD.");
  params.addRequiredParam<MooseFunctorName>("F1", "The F1 blending function.");
  return params;
}

kOmegaSSTSigmaOmegaAux::kOmegaSSTSigmaOmegaAux(const InputParameters & parameters)
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
kOmegaSSTSigmaOmegaAux::computeValue()
{
  const auto F1 = _F1(makeElemArg(_current_elem), determineState());
  const auto blend_coef = 1.0 / (F1 * _sigma_omega_1 + (1.0 - F1) * _sigma_omega_2);
  return blend_coef.value();
}
