//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCartesianMortarMechanicalContact.h"

registerMooseObject("ContactApp", KokkosCartesianMortarMechanicalContact);

static char
registerKokkosCartesianMortarMechanicalContact()
{
  using Object = KokkosCartesianMortarMechanicalContact;
  using namespace Moose::Kokkos;

  DispatcherRegistry::addTeamDispatcher<Object::MortarLoop, Object>(
      "KokkosCartesianMortarMechanicalContact");
  DispatcherRegistry::addDispatcher<Object::InactiveLoop, Object>(
      "KokkosCartesianMortarMechanicalContact");
  return 0;
}

[[maybe_unused]] static char kokkos_dispatcher_cartesian_mortar_mechanical_contact =
    registerKokkosCartesianMortarMechanicalContact();

InputParameters
KokkosCartesianMortarMechanicalContact::validParams()
{
  InputParameters params = KokkosCartesianMortarConstraintBase::validParams();
  params.addClassDescription("Applies the mortar contact traction of a Cartesian Lagrange "
                             "multiplier to all displacement components.");
  params.set<bool>("compute_lm_residuals") = false;
  return params;
}

KokkosCartesianMortarMechanicalContact::KokkosCartesianMortarMechanicalContact(
    const InputParameters & parameters)
  : KokkosCartesianMortarConstraintBase(parameters)
{
}

void
KokkosCartesianMortarMechanicalContact::dispatchMortar(const Moose::Kokkos::ThreadID n_secondary)
{
  if (!n_secondary)
    return;

  if (!_residual_dispatcher)
    _residual_dispatcher = Moose::Kokkos::DispatcherRegistry::build<MortarLoop>(this, type());

  Moose::Kokkos::TeamLaunch launch;
  launch.league_size = n_secondary;
  _residual_dispatcher->parallelForTeams(launch);
}
