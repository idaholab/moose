//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVFunctorDirichletBC.h"

registerKokkosLinearFVBoundaryCondition("MooseApp", KokkosLinearFVFunctorDirichletBC);

InputParameters
KokkosLinearFVFunctorDirichletBC::validParams()
{
  InputParameters params = Moose::Kokkos::LinearFVBoundaryCondition::validParams();
  params.addRequiredParam<FunctionName>("functor", "The boundary-value functor.");
  params.addClassDescription("Kokkos LinearFV Dirichlet boundary condition.");
  return params;
}

KokkosLinearFVFunctorDirichletBC::KokkosLinearFVFunctorDirichletBC(
    const InputParameters & parameters)
  : Moose::Kokkos::LinearFVBoundaryCondition(parameters),
    _functor(getKokkosFunction<KokkosParsedFunction>("functor"))
{
}
