//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVElementalKernel.h"

namespace Moose::Kokkos
{

InputParameters
LinearFVElementalKernel::validParams()
{
  auto params = LinearFVKernel::validParams();
  params.registerSystemAttributeName("LinearFVElementalKernel");
  return params;
}

LinearFVElementalKernel::LinearFVElementalKernel(const InputParameters & parameters)
  : LinearFVKernel(parameters)
{
}

LinearFVElementalKernel::LinearFVElementalKernel(const LinearFVElementalKernel & object)
  : LinearFVKernel(object)
{
}

void
LinearFVElementalKernel::computeRightHandSide()
{
  Policy policy(0, numKokkosBlockElements());
  if (!_rhs_dispatcher)
    _rhs_dispatcher = DispatcherRegistry::build<RightHandSideLoop>(this, type());
  _rhs_dispatcher->parallelFor(policy);
}

void
LinearFVElementalKernel::computeMatrix()
{
  if (DispatcherRegistry::hasUserMethod<MatrixLoop>(type()))
  {
    Policy policy(0, numKokkosBlockElements());
    if (!_matrix_dispatcher)
      _matrix_dispatcher = DispatcherRegistry::build<MatrixLoop>(this, type());
    _matrix_dispatcher->parallelFor(policy);
  }
}

} // namespace Moose::Kokkos
