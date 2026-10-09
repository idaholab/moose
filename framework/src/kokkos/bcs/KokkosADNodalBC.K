//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  : NodalBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD)
{
  addMooseVariableDependency(&_var);
}

void
ADNodalBC::initialSetup()
{
  NodalBCBase::initialSetup();

  _ad_setup.assign(*this, _kokkos_var, DispatcherRegistry::getProperty("max_ad_slots", type()));
  _ad_slots = _ad_setup.table();
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
