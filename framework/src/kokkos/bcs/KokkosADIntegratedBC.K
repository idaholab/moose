//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADIntegratedBC.h"

namespace Moose::Kokkos
{

InputParameters
ADIntegratedBC::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  return params;
}

ADIntegratedBC::ADIntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD), _test(), _grad_test()
{
  addMooseVariableDependency(&_var);
}

void
ADIntegratedBC::initialSetup()
{
  IntegratedBCBase::initialSetup();

  _max_ad_slots = DispatcherRegistry::getProperty("max_ad_slots", type());
  _ad_setup.assign(*this, _kokkos_var, _max_ad_slots);
  _ad_slots = _ad_setup.table();
}

void
ADIntegratedBC::computeResidual()
{
  _computing_residual = true;
  _computing_jacobian = false;

  dispatch();
}

void
ADIntegratedBC::computeJacobian()
{
  _computing_residual = false;
  _computing_jacobian = true;

  dispatch();
}

void
ADIntegratedBC::computeResidualAndJacobian()
{
  _computing_residual = true;
  _computing_jacobian = true;

  dispatch();
}

void
ADIntegratedBC::dispatch()
{
  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

  TeamLaunch launch;
  launch.league_size = numKokkosBoundarySides();
  launch.scratch_bytes = ADScratchLayout::bytes(kokkosAssembly().getMaxQpsPerElem(), _max_ad_slots);

  _residual_dispatcher->parallelForTeams(launch);
}

} // namespace Moose::Kokkos
