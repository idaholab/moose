//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVAdvectionDiffusionFunctorDirichletBC.h"

registerKokkosLinearFVBoundaryCondition("MooseApp",
                                        KokkosLinearFVAdvectionDiffusionFunctorDirichletBC);

InputParameters
KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::validParams()
{
  InputParameters params = Moose::Kokkos::LinearFVBoundaryCondition::validParams();
  params.addRequiredParam<FunctionName>("diffusion_coeff", "The diffusion coefficient.");
  params.addRequiredParam<FunctionName>("functor", "The boundary-value functor.");
  params.addClassDescription(
      "Kokkos LinearFV Dirichlet boundary condition for advection-diffusion kernels.");
  return params;
}

KokkosLinearFVAdvectionDiffusionFunctorDirichletBC::
    KokkosLinearFVAdvectionDiffusionFunctorDirichletBC(const InputParameters & parameters)
  : Moose::Kokkos::LinearFVBoundaryCondition(parameters),
    _diffusion_coeff(getKokkosFunction<KokkosParsedFunction>("diffusion_coeff")),
    _functor(getKokkosFunction<KokkosParsedFunction>("functor"))
{
}
