//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADKernel.h"

namespace Moose::Kokkos
{

InputParameters
ADKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  return params;
}

ADKernel::ADKernel(const InputParameters & parameters)
  : KernelBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD), _test(), _grad_test()
{
  addMooseVariableDependency(&_var);

  if (_configuration.displaced())
    paramError("use_displaced_mesh",
               "Kokkos AD objects do not yet differentiate with respect to the displacements of "
               "the current configuration.");
}

void
ADKernel::initialSetup()
{
  KernelBase::initialSetup();

  _max_ad_slots = DispatcherRegistry::getProperty("max_ad_slots", type());
  _ad_setup.assign(*this, _kokkos_var, _max_ad_slots);
  _ad_slots = _ad_setup.table();
}

void
ADKernel::computeResidual()
{
  _computing_residual = true;
  _computing_jacobian = false;

  dispatch();
}

void
ADKernel::computeJacobian()
{
  _computing_residual = false;
  _computing_jacobian = true;

  dispatch();
}

void
ADKernel::computeResidualAndJacobian()
{
  _computing_residual = true;
  _computing_jacobian = true;

  dispatch();
}

void
ADKernel::dispatch()
{
  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

  TeamLaunch launch;
  launch.league_size = numKokkosBlockElements();
  launch.scratch_bytes = ADScratchLayout::bytes(kokkosAssembly().getMaxQpsPerElem(), _max_ad_slots);

  _residual_dispatcher->parallelForTeams(launch);
}

} // namespace Moose::Kokkos
