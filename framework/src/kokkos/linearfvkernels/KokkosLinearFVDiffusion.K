//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVDiffusion.h"

registerKokkosLinearFVKernel("MooseApp", KokkosLinearFVDiffusion);

InputParameters
KokkosLinearFVDiffusion::validParams()
{
  InputParameters params = Moose::Kokkos::LinearFVFluxKernel::validParams();
  params.addRequiredParam<FunctionName>("diffusion_coeff", "The diffusion coefficient.");
  params.addClassDescription("Kokkos LinearFV diffusion kernel.");
  return params;
}

KokkosLinearFVDiffusion::KokkosLinearFVDiffusion(const InputParameters & parameters)
  : Moose::Kokkos::LinearFVFluxKernel(parameters),
    _diffusion_coeff(getKokkosFunction<KokkosParsedFunction>("diffusion_coeff"))
{
}
