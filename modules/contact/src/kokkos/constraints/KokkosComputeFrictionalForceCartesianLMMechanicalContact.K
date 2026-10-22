//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosComputeFrictionalForceCartesianLMMechanicalContact.h"
#include "KokkosWeightedGapUserObject.h"

#include "FEProblemBase.h"

registerMooseObject("ContactApp", KokkosComputeFrictionalForceCartesianLMMechanicalContact);

static char
registerKokkosComputeFrictionalForceCartesianLMMechanicalContact()
{
  using Object = KokkosComputeFrictionalForceCartesianLMMechanicalContact;
  using namespace Moose::Kokkos;
  const std::string name = "KokkosComputeFrictionalForceCartesianLMMechanicalContact";

  DispatcherRegistry::addDispatcher<Object::NodeLoop, Object>(name);
  DispatcherRegistry::addTeamDispatcher<Object::MortarLoop, Object>(name);
  DispatcherRegistry::addDispatcher<Object::InactiveLoop, Object>(name);
  return 0;
}

[[maybe_unused]] static char kokkos_dispatcher_frictional_force_cartesian_lm_mechanical_contact =
    registerKokkosComputeFrictionalForceCartesianLMMechanicalContact();

InputParameters
KokkosComputeFrictionalForceCartesianLMMechanicalContact::validParams()
{
  InputParameters params = KokkosCartesianMortarConstraintBase::validParams();
  params.addClassDescription("Enforces frictional mortar contact at the secondary nodes with a "
                             "Cartesian Lagrange multiplier and the Coulomb friction law.");
  params.addRequiredParam<UserObjectName>(
      "weighted_gap_uo", "The weighted gap user object, which must compute the slip.");
  params.addParam<Real>(
      "c", 1e6, "Parameter for balancing the size of the gap and contact pressure");
  params.addParam<Real>("c_t", 1, "Parameter for balancing the size of the slip and the traction");
  params.addParam<bool>(
      "normalize_c",
      false,
      "Whether to normalize c and c_t by weighting function norm. When unnormalized "
      "the value of c effectively depends on element size since in the constraint we compare nodal "
      "Lagrange Multiplier values to integrated gap values (LM nodal value is independent of "
      "element size, where integrated values are dependent on element size).");
  params.addRequiredRangeCheckedParam<Real>(
      "mu", "mu > 0", "The friction coefficient for the Coulomb friction law");
  params.addParam<Real>(
      "epsilon",
      1e-7,
      "Minimum value of contact pressure that will trigger frictional enforcement");

  // The nodal normal at a secondary node depends on every secondary face incident to the node
  params.set<bool>("ghost_point_neighbors") = true;
  params.suppressParameter<bool>("ghost_point_neighbors");

  return params;
}

KokkosComputeFrictionalForceCartesianLMMechanicalContact::
    KokkosComputeFrictionalForceCartesianLMMechanicalContact(const InputParameters & parameters)
  : KokkosCartesianMortarConstraintBase(parameters),
    _weighted_gap_uo(_fe_problem.getKokkosUserObject<KokkosWeightedGapUserObject>(
        getParam<UserObjectName>("weighted_gap_uo"))),
    _c(getParam<Real>("c")),
    _c_t(getParam<Real>("c_t")),
    _normalize_c(getParam<bool>("normalize_c")),
    _mu(getParam<Real>("mu")),
    _epsilon(getParam<Real>("epsilon"))
{
  if (!_weighted_gap_uo.parameters().get<bool>("compute_slip"))
    paramError("weighted_gap_uo", "The weighted gap user object must set compute_slip = true.");

  _interface_data.requireNodalNormals();
}

KokkosComputeFrictionalForceCartesianLMMechanicalContact::
    KokkosComputeFrictionalForceCartesianLMMechanicalContact(
        const KokkosComputeFrictionalForceCartesianLMMechanicalContact & object)
  : KokkosCartesianMortarConstraintBase(object),
    _weighted_gap_uo(object._weighted_gap_uo),
    _c(object._c),
    _c_t(object._c_t),
    _normalize_c(object._normalize_c),
    _mu(object._mu),
    _epsilon(object._epsilon),
    _gap_vector(object._gap_vector),
    _slip(object._slip),
    _normalization(object._normalization),
    _designated(object._designated)
{
}

void
KokkosComputeFrictionalForceCartesianLMMechanicalContact::dispatchMortar(
    const Moose::Kokkos::ThreadID n_secondary)
{
  if (!n_secondary)
    return;

  _gap_vector = _weighted_gap_uo.weightedGapVector();
  _slip = _weighted_gap_uo.weightedSlip();
  _normalization = _weighted_gap_uo.normalization();
  _designated = _weighted_gap_uo.designated();

  if (!_node_dispatcher)
    _node_dispatcher = Moose::Kokkos::DispatcherRegistry::build<NodeLoop>(this, type());
  _node_dispatcher->parallelFor(Moose::Kokkos::Policy(0, _geometry.nodal_normal.size()));

  // The derivatives with respect to the weighted gap vector and the slip are summed over the
  // mortar segments of the node
  if (_computing_jacobian)
  {
    if (!_residual_dispatcher)
      _residual_dispatcher = Moose::Kokkos::DispatcherRegistry::build<MortarLoop>(this, type());

    Moose::Kokkos::TeamLaunch launch;
    launch.league_size = n_secondary;
    _residual_dispatcher->parallelForTeams(launch);
  }
}
