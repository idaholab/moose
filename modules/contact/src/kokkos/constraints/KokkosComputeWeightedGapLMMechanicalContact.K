//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosComputeWeightedGapLMMechanicalContact.h"
#include "KokkosWeightedGapUserObject.h"

#include "FEProblemBase.h"

#include "libmesh/petsc_vector.h"

registerMooseObject("ContactApp", KokkosComputeWeightedGapLMMechanicalContact);

static char
registerKokkosComputeWeightedGapLMMechanicalContact()
{
  using Object = KokkosComputeWeightedGapLMMechanicalContact;
  using namespace Moose::Kokkos;

  DispatcherRegistry::addTeamDispatcher<Object::MortarLoop, Object>(
      "KokkosComputeWeightedGapLMMechanicalContact");
  DispatcherRegistry::addDispatcher<Object::NodeLoop, Object>(
      "KokkosComputeWeightedGapLMMechanicalContact");
  DispatcherRegistry::addDispatcher<Object::InactiveLoop, Object>(
      "KokkosComputeWeightedGapLMMechanicalContact");
  return 0;
}

[[maybe_unused]] static char kokkos_dispatcher_compute_weighted_gap_lm_mechanical_contact =
    registerKokkosComputeWeightedGapLMMechanicalContact();

InputParameters
KokkosComputeWeightedGapLMMechanicalContact::validParams()
{
  InputParameters params = MortarConstraintBase::validParams();
  params.addClassDescription("Enforces the frictionless mortar contact conditions at the secondary "
                             "nodes with a nonlinear complementarity function of the weighted gap "
                             "and the normal Lagrange multiplier.");
  params.addRequiredParam<UserObjectName>("weighted_gap_uo", "The weighted gap user object");
  params.addParam<Real>(
      "c", 1e6, "Parameter for balancing the size of the gap and contact pressure");
  params.addParam<bool>(
      "normalize_c",
      false,
      "Whether to normalize c by weighting function norm. When unnormalized "
      "the value of c effectively depends on element size since in the constraint we compare nodal "
      "Lagrange Multiplier values to integrated gap values (LM nodal value is independent of "
      "element size, where integrated values are dependent on element size).");

  // The constraint is posed on the current configuration, and the nodal normal at a secondary node
  // depends on every secondary face incident to the node
  params.set<bool>("use_displaced_mesh") = true;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.set<bool>("ghost_point_neighbors") = true;
  params.suppressParameter<bool>("ghost_point_neighbors");
  params.set<bool>("interpolate_normals") = false;
  params.suppressParameter<bool>("interpolate_normals");

  return params;
}

KokkosComputeWeightedGapLMMechanicalContact::KokkosComputeWeightedGapLMMechanicalContact(
    const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _weighted_gap_uo(_fe_problem.getKokkosUserObject<KokkosWeightedGapUserObject>(
        getParam<UserObjectName>("weighted_gap_uo"))),
    _c(getParam<Real>("c")),
    _normalize_c(getParam<bool>("normalize_c"))
{
  if (_lm_family != libMesh::LAGRANGE || _lm_order != libMesh::FIRST)
    paramError("variable",
               "The Lagrange multiplier must be a first-order Lagrange variable, whose degrees of "
               "freedom are at the secondary nodes.");

  _interface_data.requireNodalNormals();
}

KokkosComputeWeightedGapLMMechanicalContact::KokkosComputeWeightedGapLMMechanicalContact(
    const KokkosComputeWeightedGapLMMechanicalContact & object)
  : MortarConstraintBase(object),
    _weighted_gap_uo(object._weighted_gap_uo),
    _c(object._c),
    _normalize_c(object._normalize_c),
    _gap(object._gap),
    _normalization(object._normalization),
    _owned_gap(object._owned_gap),
    _owned_normalization(object._owned_normalization),
    _owned_dofs(object._owned_dofs)
{
}

void
KokkosComputeWeightedGapLMMechanicalContact::dispatchMortar(
    const Moose::Kokkos::ThreadID n_secondary)
{
  _gap = _weighted_gap_uo.weightedGap();
  _normalization = _weighted_gap_uo.normalization();
  _owned_gap = _weighted_gap_uo.ownedWeightedGap();
  _owned_normalization = _weighted_gap_uo.ownedNormalization();

  if (_weighted_gap_uo.ownedRevision() != _owned_revision)
  {
    const auto & mesh = _mesh.getMesh();
    const auto & solution =
        static_cast<const libMesh::PetscVector<Number> &>(*_sys.system().current_local_solution);
    const auto & nodes = _weighted_gap_uo.ownedNodes();

    _owned_dofs.create(nodes.size());
    for (const auto i : index_range(nodes))
      _owned_dofs[i] = solution.map_global_to_local_index(
          mesh.node_ref(nodes[i]).dof_number(_sys.number(), _var.number(), 0));
    _owned_dofs.copyToDevice();
    _owned_revision = _weighted_gap_uo.ownedRevision();
  }

  if (n_secondary)
  {
    if (!_residual_dispatcher)
      _residual_dispatcher = Moose::Kokkos::DispatcherRegistry::build<MortarLoop>(this, type());

    Moose::Kokkos::TeamLaunch launch;
    launch.league_size = n_secondary;
    _residual_dispatcher->parallelForTeams(launch);
  }

  if (_owned_dofs.size())
  {
    if (!_node_dispatcher)
      _node_dispatcher = Moose::Kokkos::DispatcherRegistry::build<NodeLoop>(this, type());

    _node_dispatcher->parallelFor(Moose::Kokkos::Policy(0, _owned_dofs.size()));
  }
}
