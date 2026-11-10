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
  _ad_setup.assign(*this, _kokkos_var, _max_ad_slots, _configuration.displacements(), kokkosMesh());
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
  const auto n_items = numKokkosBoundarySides();
  const auto max_qps = kokkosAssembly().getMaxQpsPerSide();

  // A residual alone needs no derivatives, so it evaluates the hooks with plain reals
  if (!_computing_jacobian)
  {
    if (!_residual_only_dispatcher)
      _residual_only_dispatcher = DispatcherRegistry::build<ResidualOnlyLoop>(this, type());

    dispatchADTeams(*_residual_only_dispatcher, n_items, max_qps, 0, _ad_slots);
  }
  // An object posed on the current configuration, or reading a property that depends on the
  // displacements, dispatches the instantiation whose dual numbers also carry the displacement
  // slots, provided that a displacement component is seeded
  else if (_ad_slots.hasDisplacements())
  {
    if (!_displaced_residual_dispatcher)
      _displaced_residual_dispatcher =
          DispatcherRegistry::build<DisplacedResidualLoop>(this, type());

    dispatchADTeams(*_displaced_residual_dispatcher,
                    n_items,
                    max_qps,
                    _max_ad_slots + AD_DISPLACEMENT_SLOTS,
                    _ad_slots);
  }
  else
  {
    if (!_residual_dispatcher)
      _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

    dispatchADTeams(*_residual_dispatcher, n_items, max_qps, _max_ad_slots, _ad_slots);
  }
}

} // namespace Moose::Kokkos
