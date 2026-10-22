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
  params.addParam<bool>("compute_slip",
                        false,
                        "Whether to compute the weighted relative displacement increment over the "
                        "time step, which frictional contact reads.");

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
    _use_dual(_lm_var.useDual()),
    _compute_slip(getParam<bool>("compute_slip"))
{
  if (_lm_family != libMesh::LAGRANGE || _lm_order != libMesh::FIRST)
    paramError("lm_variable",
               "The Lagrange multiplier must be a first-order Lagrange variable, whose degrees of "
               "freedom are at the secondary nodes.");

  if (_compute_slip)
  {
    _disp = kokkosCoupledVectorTagValuesByName("_displacements", Moose::SOLUTION_TAG).variable();
    _disp_old =
        kokkosCoupledVectorTagValuesByName("_displacements", Moose::OLD_SOLUTION_TAG).variable();
    const auto & fe_type = _disp.mooseFieldVar()->feType();
    _disp_family = fe_type.family;
    _disp_order = fe_type.order;
  }

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
    _compute_slip(object._compute_slip),
    _disp(object._disp),
    _disp_old(object._disp_old),
    _disp_fe(object._disp_fe),
    _disp_family(object._disp_family),
    _disp_order(object._disp_order),
    _gap_vector(object._gap_vector),
    _normalization(object._normalization),
    _slip(object._slip)
{
}

void
KokkosWeightedGapUserObject::compute()
{
  _geometry = _interface_data.update();
  _lm_fe = kokkosAssembly().getFETypeID(_lm_var.feType());
  if (_compute_slip)
    _disp_fe = kokkosAssembly().getFETypeID(_disp.mooseFieldVar()->feType());

  const auto n_normals = _interface_data.normalNodes().size();
  _gap_vector.create(n_normals);
  _normalization.create(n_normals);
  _slip.create(_compute_slip ? n_normals : 0);
  _gap_vector = Moose::Kokkos::Real3(0);
  _normalization = 0;
  _slip = Moose::Kokkos::Real3(0);

  if (_geometry.secondary_qp_begin.size() > 1)
  {
    if (!_gap_dispatcher)
      _gap_dispatcher = Moose::Kokkos::DispatcherRegistry::build<GapLoop>(this, type());

    Moose::Kokkos::TeamLaunch launch;
    launch.league_size = _geometry.secondary_qp_begin.size() - 1;
    _gap_dispatcher->parallelForTeams(launch);
  }

  _gap_vector.copyToHost();
  _normalization.copyToHost();
  _slip.copyToHost();

  communicate();
}

void
KokkosWeightedGapUserObject::communicate()
{
  const auto & nodes = _interface_data.normalNodes();
  const auto rank = processor_id();

  // The nodal sums, packed as the gap vector, the normalization, the slip, and the weighted gap,
  // which every process computes with the same nodal normal
  constexpr unsigned int n_sums = 8;
  using Sums = std::array<Real, n_sums>;
  const auto pack = [&](const dof_id_type i)
  {
    Sums v{};
    for (const auto c : make_range(3u))
      v[c] = _gap_vector[i](c);
    v[3] = _normalization[i];
    if (_compute_slip)
      for (const auto c : make_range(3u))
        v[4 + c] = _slip[i](c);
    v[7] = _gap_vector[i] * _geometry.nodal_normal[i];
    return v;
  };
  const auto add = [](Sums & sums, const Sums & v)
  {
    for (const auto k : make_range(n_sums))
      sums[k] += v[k];
  };

  // The owner of a node sums the contributions of all processes and records the processes whose
  // local secondary elements touch the node, to send them the sums and to designate one of them
  std::map<dof_id_type, Sums> owned;
  std::map<dof_id_type, processor_id_type> designee;
  using Datum = std::pair<dof_id_type, Sums>;
  std::map<processor_id_type, std::vector<Datum>> push;

  for (const auto i : index_range(nodes))
  {
    const auto node = nodes[i];
    if (node->processor_id() == rank)
    {
      add(owned[node->id()], pack(i));
      designee[node->id()] = rank;
    }
    else
      push[node->processor_id()].emplace_back(node->id(), pack(i));
  }

  std::map<processor_id_type, std::vector<dof_id_type>> contributors;
  auto receive = [&](const processor_id_type pid, const std::vector<Datum> & data)
  {
    for (const auto & [id, v] : data)
    {
      add(owned[id], v);
      contributors[pid].push_back(id);
      // The owner assembles the nodes it touches, and the lowest touching process the others
      const auto [it, inserted] = designee.emplace(id, pid);
      if (!inserted && it->second != rank)
        it->second = std::min(it->second, pid);
    }
  };
  TIMPI::push_parallel_vector_data(comm(), push, receive);

  // The last entry of a reply is 1 for the designated process of the node
  using Reply = std::pair<dof_id_type, std::array<Real, n_sums + 1>>;
  const auto make_reply = [](const Sums & v, const bool designated)
  {
    std::array<Real, n_sums + 1> r;
    std::copy(v.begin(), v.end(), r.begin());
    r[n_sums] = designated;
    return r;
  };
  std::map<processor_id_type, std::vector<Reply>> reply;
  for (const auto & [pid, ids] : contributors)
    for (const auto id : ids)
      reply[pid].emplace_back(
          id, make_reply(libmesh_map_find(owned, id), libmesh_map_find(designee, id) == pid));

  std::unordered_map<dof_id_type, std::pair<Sums, bool>> remote;
  auto receive_reply = [&](const processor_id_type, const std::vector<Reply> & data)
  {
    for (const auto & [id, r] : data)
    {
      Sums v;
      std::copy(r.begin(), r.begin() + n_sums, v.begin());
      remote[id] = {v, r[n_sums] != 0};
    }
  };
  TIMPI::push_parallel_vector_data(comm(), reply, receive_reply);

  _gap.create(nodes.size());
  _designated.create(nodes.size());
  for (const auto i : index_range(nodes))
  {
    const auto id = nodes[i]->id();
    const bool own = nodes[i]->processor_id() == rank;
    const auto & v = own ? libmesh_map_find(owned, id) : libmesh_map_find(remote, id).first;
    for (const auto c : make_range(3u))
      _gap_vector[i](c) = v[c];
    _normalization[i] = v[3];
    if (_compute_slip)
      for (const auto c : make_range(3u))
        _slip[i](c) = v[4 + c];
    _gap[i] = v[7];
    _designated[i] =
        own ? libmesh_map_find(designee, id) == rank : libmesh_map_find(remote, id).second;
  }
  _gap.copyToDevice();
  _gap_vector.copyToDevice();
  _normalization.copyToDevice();
  _slip.copyToDevice();
  _designated.copyToDevice();

  std::vector<dof_id_type> owned_nodes;
  owned_nodes.reserve(owned.size());
  for (const auto & [id, _] : owned)
    owned_nodes.push_back(id);
  if (owned_nodes != _owned_nodes)
  {
    _owned_nodes = std::move(owned_nodes);
    ++_owned_revision;
  }

  _owned_gap.create(_owned_nodes.size());
  _owned_normalization.create(_owned_nodes.size());
  dof_id_type k = 0;
  for (const auto & [id, v] : owned)
  {
    _owned_gap[k] = v[7];
    _owned_normalization[k] = v[3];
    ++k;
  }
  _owned_gap.copyToDevice();
  _owned_normalization.copyToDevice();
}
