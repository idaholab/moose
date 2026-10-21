//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosWeightedGapUserObject.h"

#include "AutomaticMortarGeneration.h"
#include "FEProblemBase.h"

#include "timpi/parallel_sync.h"

registerMooseObject("ContactApp", KokkosWeightedGapUserObject);

static char
registerKokkosWeightedGapUserObject()
{
  Moose::Kokkos::DispatcherRegistry::addTeamDispatcher<KokkosWeightedGapUserObject::GapLoop,
                                                       KokkosWeightedGapUserObject>(
      "KokkosWeightedGapUserObject");
  return 0;
}

[[maybe_unused]] static char kokkos_dispatcher_weighted_gap_user_object =
    registerKokkosWeightedGapUserObject();

InputParameters
KokkosWeightedGapUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += MortarConsumerInterface::validParams();
  params.addClassDescription("Computes the weighted gap of each secondary node of a mortar "
                             "contact interface on the current configuration.");
  params.addRequiredParam<VariableName>(
      "lm_variable", "The Lagrange multiplier whose basis weights the gap, the contact pressure.");

  // The gap is the distance between the faces of the current configuration
  params.set<bool>("use_displaced_mesh") = true;
  params.suppressParameter<bool>("use_displaced_mesh");
  // The segments of a partially covered secondary element are integrated
  params.set<bool>("correct_edge_dropping") = true;
  params.suppressParameter<bool>("correct_edge_dropping");
  params.set<bool>("interpolate_normals") = false;
  params.suppressParameter<bool>("interpolate_normals");

  // The contact constraints read the gaps while assembling every residual and Jacobian
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

KokkosWeightedGapUserObject::KokkosWeightedGapUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _interface(std::make_shared<Moose::Kokkos::MortarConsumer>(this)),
    _interface_data(_fe_problem.getKokkosMortarInterfaceData(_interface->amg())),
    _lm_var(_fe_problem.getVariable(_tid, getParam<VariableName>("lm_variable"))),
    _lm_family(_lm_var.feType().family),
    _lm_order(_lm_var.feType().order),
    _use_dual(_lm_var.useDual())
{
  if (_lm_family != libMesh::LAGRANGE || _lm_order != libMesh::FIRST)
    paramError("lm_variable",
               "The Lagrange multiplier must be a first-order Lagrange variable, whose degrees of "
               "freedom are at the secondary nodes.");

  _interface_data.requireNodalNormals();
}

KokkosWeightedGapUserObject::KokkosWeightedGapUserObject(const KokkosWeightedGapUserObject & object)
  : GeneralUserObject(object),
    _interface(object._interface),
    _interface_data(object._interface_data),
    _geometry(object._geometry),
    _lm_var(object._lm_var),
    _lm_fe(object._lm_fe),
    _lm_family(object._lm_family),
    _lm_order(object._lm_order),
    _use_dual(object._use_dual),
    _gap(object._gap),
    _normalization(object._normalization)
{
}

void
KokkosWeightedGapUserObject::compute()
{
  _geometry = _interface_data.update();
  _lm_fe = kokkosAssembly().getFETypeID(_lm_var.feType());

  const auto n_normals = _interface_data.normalNodes().size();
  _gap.create(n_normals);
  _normalization.create(n_normals);
  _gap = 0;
  _normalization = 0;

  if (_geometry.secondary_qp_begin.size() > 1)
  {
    if (!_gap_dispatcher)
      _gap_dispatcher = Moose::Kokkos::DispatcherRegistry::build<GapLoop>(this, type());

    Moose::Kokkos::TeamLaunch launch;
    launch.league_size = _geometry.secondary_qp_begin.size() - 1;
    _gap_dispatcher->parallelForTeams(launch);
  }

  _gap.copyToHost();
  _normalization.copyToHost();

  communicate();
}

void
KokkosWeightedGapUserObject::communicate()
{
  const auto & nodes = _interface_data.normalNodes();
  const auto rank = processor_id();

  // The sums at the owned nodes, starting from the contributions of this process
  std::map<dof_id_type, std::pair<Real, Real>> owned;
  using Datum = std::tuple<dof_id_type, Real, Real>;
  std::map<processor_id_type, std::vector<Datum>> push;

  for (const auto i : index_range(nodes))
  {
    const auto node = nodes[i];
    if (node->processor_id() == rank)
    {
      auto & [gap, normalization] = owned[node->id()];
      gap += _gap[i];
      normalization += _normalization[i];
    }
    else
      push[node->processor_id()].emplace_back(node->id(), _gap[i], _normalization[i]);
  }

  // The owner of a node also records which processes contributed to it, to send them the sums
  std::map<processor_id_type, std::vector<dof_id_type>> contributors;
  auto receive = [&](const processor_id_type pid, const std::vector<Datum> & data)
  {
    for (const auto & [id, gap, normalization] : data)
    {
      auto & sums = owned[id];
      sums.first += gap;
      sums.second += normalization;
      contributors[pid].push_back(id);
    }
  };
  TIMPI::push_parallel_vector_data(comm(), push, receive);

  std::map<processor_id_type, std::vector<Datum>> reply;
  for (const auto & [pid, ids] : contributors)
    for (const auto id : ids)
    {
      const auto & [gap, normalization] = libmesh_map_find(owned, id);
      reply[pid].emplace_back(id, gap, normalization);
    }

  std::unordered_map<dof_id_type, std::pair<Real, Real>> remote;
  auto receive_reply = [&](const processor_id_type, const std::vector<Datum> & data)
  {
    for (const auto & [id, gap, normalization] : data)
      remote[id] = {gap, normalization};
  };
  TIMPI::push_parallel_vector_data(comm(), reply, receive_reply);

  for (const auto i : index_range(nodes))
  {
    const auto id = nodes[i]->id();
    const auto & sums = nodes[i]->processor_id() == rank ? libmesh_map_find(owned, id)
                                                         : libmesh_map_find(remote, id);
    _gap[i] = sums.first;
    _normalization[i] = sums.second;
  }
  _gap.copyToDevice();
  _normalization.copyToDevice();

  std::vector<dof_id_type> owned_nodes;
  owned_nodes.reserve(owned.size());
  for (const auto & [id, sums] : owned)
    owned_nodes.push_back(id);
  if (owned_nodes != _owned_nodes)
  {
    _owned_nodes = std::move(owned_nodes);
    ++_owned_revision;
  }

  _owned_gap.create(_owned_nodes.size());
  _owned_normalization.create(_owned_nodes.size());
  dof_id_type k = 0;
  for (const auto & [id, sums] : owned)
  {
    _owned_gap[k] = sums.first;
    _owned_normalization[k] = sums.second;
    ++k;
  }
  _owned_gap.copyToDevice();
  _owned_normalization.copyToDevice();
}
