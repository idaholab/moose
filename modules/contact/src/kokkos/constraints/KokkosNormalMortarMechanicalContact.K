//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNormalMortarMechanicalContact.h"

#include "FEProblemBase.h"

registerMooseObject("ContactApp", KokkosNormalMortarMechanicalContact);

static char
registerKokkosNormalMortarMechanicalContact()
{
  using Object = KokkosNormalMortarMechanicalContact;
  using namespace Moose::Kokkos;

  DispatcherRegistry::addTeamDispatcher<Object::MortarLoop, Object>(
      "KokkosNormalMortarMechanicalContact");
  DispatcherRegistry::addDispatcher<Object::InactiveLoop, Object>(
      "KokkosNormalMortarMechanicalContact");
  return 0;
}

[[maybe_unused]] static char kokkos_dispatcher_normal_mortar_mechanical_contact =
    registerKokkosNormalMortarMechanicalContact();

InputParameters
KokkosNormalMortarMechanicalContact::validParams()
{
  InputParameters params = MortarConstraintBase::validParams();
  params.addClassDescription("Applies the normal mortar contact force of a Lagrange multiplier to "
                             "all displacement components.");

  // The force acts on the displacements of the current configuration and the multiplier rows
  // belong to the constraint that enforces the contact conditions
  params.set<bool>("use_displaced_mesh") = true;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.set<bool>("compute_lm_residuals") = false;
  params.set<bool>("ghost_point_neighbors") = true;
  params.suppressParameter<bool>("ghost_point_neighbors");
  params.set<bool>("interpolate_normals") = false;
  params.suppressParameter<bool>("interpolate_normals");

  return params;
}

KokkosNormalMortarMechanicalContact::KokkosNormalMortarMechanicalContact(
    const InputParameters & parameters)
  : MortarConstraintBase(parameters)
{
  if (_lm_family != libMesh::LAGRANGE || _lm_order != libMesh::FIRST)
    paramError("variable",
               "The Lagrange multiplier must be a first-order Lagrange variable, whose degrees of "
               "freedom are at the secondary nodes.");
  if (!_n_disp)
    paramError("use_displaced_mesh",
               "The displacements of the [Mesh] block must be solution variables of the system of "
               "the Lagrange multiplier.");

  for (const auto c : make_range(3u))
    _lm_coupled[c] = c < _n_disp && _disp_var[c].initialized() &&
                     _fe_problem.areCoupled(_disp_var[c].var(), _var.number(), _sys.number());

  _interface_data.requireNodalNormals();
}

KokkosNormalMortarMechanicalContact::KokkosNormalMortarMechanicalContact(
    const KokkosNormalMortarMechanicalContact & object)
  : MortarConstraintBase(object)
{
  for (const auto c : make_range(3u))
    _lm_coupled[c] = object._lm_coupled[c];
}

void
KokkosNormalMortarMechanicalContact::dispatchMortar(const Moose::Kokkos::ThreadID n_secondary)
{
  if (!n_secondary)
    return;

  if (!_residual_dispatcher)
    _residual_dispatcher = Moose::Kokkos::DispatcherRegistry::build<MortarLoop>(this, type());

  Moose::Kokkos::TeamLaunch launch;
  launch.league_size = n_secondary;
  _residual_dispatcher->parallelForTeams(launch);
}
