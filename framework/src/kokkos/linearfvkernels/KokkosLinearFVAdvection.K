//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVAdvection.h"

registerKokkosLinearFVKernel("MooseApp", KokkosLinearFVAdvection);

InputParameters
KokkosLinearFVAdvection::validParams()
{
  InputParameters params = Moose::Kokkos::LinearFVFluxKernel::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity.");
  params.addClassDescription("Kokkos LinearFV first-order upwind advection kernel.");
  return params;
}

KokkosLinearFVAdvection::KokkosLinearFVAdvection(const InputParameters & parameters)
  : Moose::Kokkos::LinearFVFluxKernel(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}
