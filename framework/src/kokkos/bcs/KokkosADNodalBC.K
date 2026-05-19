//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADNodalBC.h"

namespace Moose::Kokkos
{

InputParameters
ADNodalBC::validParams()
{
  InputParameters params = NodalBCBase::validParams();
  return params;
}

ADNodalBC::ADNodalBC(const InputParameters & parameters)
  : NodalBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(_var, Moose::SOLUTION_TAG, true)
{
  addMooseVariableDependency(&_var);
}

void
ADNodalBC::computeResidual()
{
  _computing_residual = true;
  _computing_jacobian = false;

  dispatch();
}

void
ADNodalBC::computeJacobian()
{
  _computing_residual = false;
  _computing_jacobian = true;

  dispatch();
}

void
ADNodalBC::computeResidualAndJacobian()
{
  _computing_residual = true;
  _computing_jacobian = true;

  dispatch();
}

void
ADNodalBC::dispatch()
{
  Policy policy(0, numKokkosBoundaryNodes());

  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

  _residual_dispatcher->parallelFor(policy);
}

} // namespace Moose::Kokkos
