//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxLimiter.h"
#include "NonlinearSystem.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("PorousFlowApp", FluxLimiter);

template <>
InputParameters
validParams<FluxLimiter>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Kuzmin-Turek flux limiting stuff.  ONLY WORKS FOR 1D");
  params.addRequiredCoupledVar("u", "The variable that is being advected");
  MooseEnum flux_limiter_type("MinMod VanLeer MC superbee None", "MinMod");
  params.addParam<MooseEnum>("flux_limiter_type", flux_limiter_type, "Type of flux limiter to use");
  // TODO: do we need timestep_begin, or just nonlinear?
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_TIMESTEP_BEGIN, EXEC_NONLINEAR};
  return params;
}

FluxLimiter::FluxLimiter(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    Coupleable(this, false),
    _u_nodal(getVar("u", 0)),
    _u_var_num(coupled("u", 0)),
    _flux_limiter_type(getParam<MooseEnum>("flux_limiter_type").getEnum<FluxLimiterTypeEnum>()),
    _mesh(_subproblem.mesh()),
    _nodes_to_elem_map(0)
{
  // TODO: test this
  if (!_execute_enum.contains(EXEC_TIMESTEP_BEGIN) || !_execute_enum.contains(EXEC_NONLINEAR))
    mooseError("The FluxLimiter UserObject " + name() +
               " execute_on parameter must include, at least, 'timestep_begin nonlinear'");
}

void
FluxLimiter::execute()
{
  // TODO: This UserObject should have execute_on = 'nonlinear' because we need to compute k_ij
  // every nonlinear iteration.
  computeKij();
}

void
FluxLimiter::timestepSetup()
{
  if (_nodes_to_elem_map.size() == 0)
  {
    // TODO: We don't want to recompute the neighboring nodes every nonlinear iteration.  We
    // only need to do that at the beginning and when the mesh has changed because of adaptivity
    // (and anything else i haven't thought about?)
    // TODO: does this properly account for multiple processors?
    // TODO: or do we want to use _mesh.nodeToElemMap() ?   (if so, then
    // MeshTools::find_nodal_neighbors can't be used?)
    // TODO: threading?
    _nodes_to_elem_map.clear();
    MeshTools::build_nodes_to_elem_map(_mesh, _nodes_to_elem_map);
  }
}

void
FluxLimiter::meshChanged()
{
  // TODO: rebuild the _nodes_to_elem_map when adaptivity has caused a changed
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh, _nodes_to_elem_map);
}

Real
FluxLimiter::val_at_node(const Node & node) const
{
  Moose::out << "node id = " << node.unique_id() << " connected to:";
  std::vector<const Node *> neighbors;
  MeshTools::find_nodal_neighbors(_mesh, node, _nodes_to_elem_map, neighbors);
  for (const auto & n : neighbors)
    Moose::out << " " << n->unique_id();
  Moose::out << "\n";

  // This is fine:
  // return _u_nodal->getNodalValue(node);
  // This is more convoluted, but it is used below in pPlus, etc
  dof_id_type dof = node.dof_number(_u_nodal->sys().number(), _u_var_num, 0);
  const NumericVector<Real> & u = *_u_nodal->sys().currentSolution();
  return u(dof);
}

Real
FluxLimiter::rPlus(const Node & node_i) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  const Real p = pPlus(node_i);
  const Real q = qPlus(node_i);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  const Real r = q / p;
  return limitFlux(1.0, r);
}

Real
FluxLimiter::rMinus(const Node & node_i) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;
  const Real p = pMinus(node_i);
  if (p == 0.0)
    // Comment after Eqn (49): if P=0 then there's no antidiffusion, so no need to remove it
    return 1.0;
  const Real q = qMinus(node_i);
  const Real r = q / p;
  return limitFlux(1.0, r);
}

Real
FluxLimiter::pPlus(const Node & node_i) const
{
  const Real u_i = _u_nodal->getNodalValue(node_i);

  Real result = 0.0;

  // Sum over all nodes connected with node_i.
  std::vector<const Node *> neighbor_nodes;
  MeshTools::find_nodal_neighbors(_mesh, node_i, _nodes_to_elem_map, neighbor_nodes);
  for (const auto & node_j : neighbor_nodes)
  {
    const Real u_j = _u_nodal->getNodalValue(*node_j);
    result += std::min(0.0, getKij(node_i, *node_j)) * std::min(0.0, u_j - u_i);
  }

  return result;
}

Real
FluxLimiter::pMinus(const Node & node_i) const
{
  const Real u_i = _u_nodal->getNodalValue(node_i);

  Real result = 0.0;

  // Sum over all nodes connected with node_i.
  std::vector<const Node *> neighbor_nodes;
  MeshTools::find_nodal_neighbors(_mesh, node_i, _nodes_to_elem_map, neighbor_nodes);
  for (const auto & node_j : neighbor_nodes)
  {
    const Real u_j = _u_nodal->getNodalValue(*node_j);
    result += std::min(0.0, getKij(node_i, *node_j)) * std::max(0.0, u_j - u_i);
  }

  return result;
}

Real
FluxLimiter::qPlus(const Node & node_i) const
{
  const Real u_i = _u_nodal->getNodalValue(node_i);

  Real result = 0.0;

  // Sum over all nodes connected with node_i.
  std::vector<const Node *> neighbor_nodes;
  MeshTools::find_nodal_neighbors(_mesh, node_i, _nodes_to_elem_map, neighbor_nodes);
  for (const auto & node_j : neighbor_nodes)
  {
    const Real u_j = _u_nodal->getNodalValue(*node_j);
    result += std::max(0.0, getKij(node_i, *node_j)) * std::max(0.0, u_j - u_i);
  }

  return result;
}

Real
FluxLimiter::qMinus(const Node & node_i) const
{
  const Real u_i = _u_nodal->getNodalValue(node_i);

  Real result = 0.0;

  // Sum over all nodes connected with node_i.
  std::vector<const Node *> neighbor_nodes;
  MeshTools::find_nodal_neighbors(_mesh, node_i, _nodes_to_elem_map, neighbor_nodes);
  for (const auto & node_j : neighbor_nodes)
  {
    const Real u_j = _u_nodal->getNodalValue(*node_j);
    result += std::max(0.0, getKij(node_i, *node_j)) * std::min(0.0, u_j - u_i);
  }

  return result;
}

Real
FluxLimiter::limitFlux(Real a, Real b) const
{
  if (_flux_limiter_type == FluxLimiterTypeEnum::None)
    return 0.0;

  if ((a >= 0.0 && b <= 0.0) || (a <= 0.0 && b >= 0.0))
    return 0.0;
  const Real s = (a > 0.0 ? 1.0 : -1.0);

  const Real lal = std::abs(a);
  const Real lbl = std::abs(b);
  switch (_flux_limiter_type)
  {
    case FluxLimiterTypeEnum::MinMod:
      return s * std::min(lal, lbl);
    case FluxLimiterTypeEnum::VanLeer:
      return s * 2 * lal * lbl / (lal + lbl);
    case FluxLimiterTypeEnum::MC:
      return s * std::min(0.5 * std::abs(a + b), 2.0 * std::min(lal, lbl));
    case FluxLimiterTypeEnum::superbee:
      return s * std::max(std::min(2.0 * lal, lbl), std::min(lal, 2.0 * lbl));
    default:
      mooseError("Unimplemented FluxLimiter type");
  }
  return 0.0;
}

void
FluxLimiter::initializeKij()
{
  // for all the nodes we can see, set _kij to zero
  _kij = {};

  // TODO: does the following loop run through nodes on this processor only (+ ghosted ones?) or all
  // nodes?  We only want nodes on this processor
  for (const auto & node_i : _mesh.getMesh().node_ptr_range())
  {
    std::map<Node, Real> nodal_flux = {};
    std::vector<const Node *> neighbors;
    MeshTools::find_nodal_neighbors(_mesh, *node_i, _nodes_to_elem_map, neighbors);
    for (const auto & n : neighbors)
      nodal_flux.emplace(*n, 0.0);
    nodal_flux.emplace(*node_i, 0.0); // include node_i - node_i connection
    _kij.emplace(*node_i, nodal_flux);
  }
}

void
FluxLimiter::computeKij()
{
  initializeKij();

  // TODO: should this be active_local_element_ptr_range() ?
  for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
  {
    for (const auto & node_i : elem->node_ref_range())
    {
      for (const auto & node_j : elem->node_ref_range())
      {
        // TODO: The following is true for the 1D case only
        if (node_i.unique_id() == node_j.unique_id() + 1)
          _kij[node_i][node_j] += 0.5;
        else if (node_i.unique_id() == node_j.unique_id() - 1)
          _kij[node_i][node_j] -= 0.5;
      }
    }
  }

  // TODO: want this kind of thing instead, which, in 1D with _velocity=1, yields the above
  // for (unsigned i = 0; i < num_nodes; ++i)
  //{
  //  for (unsigned j = 0; j < num_nodes; ++j)
  //    for (unsigned qp = 0; qp < _qrule->n_points(); ++qp)
  //      kk[i][j] += _JxW[qp] * _coord[qp] * (_grad_test[i][qp] * _velocity) * _test[j][qp];
  //}
}

Real
FluxLimiter::getKij(const Node & node_i, const Node & node_j) const
{
  const auto & row_find = _kij.find(node_i);
  mooseAssert(row_find != _kij.end(),
              "FluxLimiter UserObject " << name() << " Kij does not contain node "
                                        << node_i.unique_id());
  const std::map<Node, Real> & kij_row = row_find->second;
  const auto & entry_find = kij_row.find(node_j);
  mooseAssert(entry_find != kij_row.end(),
              "FluxLimiter UserObject " << name() << " Kij on row " << node_i.unique_id()
                                        << " does not contain node " << node_j.unique_id());

  return entry_find->second;
}
