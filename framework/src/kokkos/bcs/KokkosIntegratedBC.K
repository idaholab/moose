//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosIntegratedBC.h"

namespace Moose::Kokkos
{

InputParameters
IntegratedBC::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  return params;
}

IntegratedBC::IntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _test(),
    _grad_test(),
    _phi(),
    _grad_phi(),
    _u(_var),
    _grad_u(_var)
{
  addMooseVariableDependency(&_var);
}

void
IntegratedBC::computeResidual()
{
  computePrecomputedResidual(numKokkosBoundarySides(), kokkosAssembly().getMaxQpsPerSide());
}

void
IntegratedBC::computeJacobian()
{
  computePrecomputedJacobian(
      numKokkosBoundarySides(), kokkosAssembly().getMaxQpsPerSide(), supports_scalar_jacobian);
}

} // namespace Moose::Kokkos
