//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVAdvectionDiffusionFunctorNeumannBC.h"

registerKokkosLinearFVBoundaryCondition("MooseApp",
                                        KokkosLinearFVAdvectionDiffusionFunctorNeumannBC);

InputParameters
KokkosLinearFVAdvectionDiffusionFunctorNeumannBC::validParams()
{
  InputParameters params = Moose::Kokkos::LinearFVBoundaryCondition::validParams();
  params.addRequiredParam<FunctionName>("functor",
                                        "The outward normal flux function on the boundary.");
  params.addClassDescription(
      "Kokkos LinearFV Neumann boundary condition for advection-diffusion kernels. "
      "Applies a prescribed outward normal flux (positive = outflow).");
  return params;
}

KokkosLinearFVAdvectionDiffusionFunctorNeumannBC::KokkosLinearFVAdvectionDiffusionFunctorNeumannBC(
    const InputParameters & parameters)
  : Moose::Kokkos::LinearFVBoundaryCondition(parameters),
    _flux_functor(getKokkosFunction<KokkosParsedFunction>("functor"))
{
}
