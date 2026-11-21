//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGradientMagnitudeAux.h"

registerKokkosAuxKernel("KokkosTrainingApp", KokkosGradientMagnitudeAux);

InputParameters
KokkosGradientMagnitudeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Magnitude of the gradient of a coupled variable.");
  params.addRequiredCoupledVar("v", "The variable whose gradient magnitude is computed");
  return params;
}

KokkosGradientMagnitudeAux::KokkosGradientMagnitudeAux(const InputParameters & parameters)
  : AuxKernel(parameters), _grad_v(kokkosCoupledGradient("v"))
{
  if (isNodal())
    paramError("variable", "The gradient is not defined at nodes; use an elemental variable.");
}
