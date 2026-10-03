//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LaplaceSmoother.h"

#include "FEProblemBase.h"
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"

#include "libmesh/boundary_info.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/enum_convergence_flags.h"
#include "libmesh/equation_systems.h"
#include "libmesh/fe_base.h"
#include "libmesh/int_range.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/linear_solver.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/utility.h"

using namespace libMesh;

registerMooseObject("MooseApp", LaplaceSmoother);

InputParameters
LaplaceSmoother::validParams()
{
  InputParameters params = MeshSmootherBase::validParams();
  params += FunctionInterface::validParams();

  params.addClassDescription(
      "Moves the mesh between remesh events by solving a Laplace equation on the configuration "
      "snapshotted at the last remesh, which harmonically interpolates the prescribed motion of "
      "the moving boundaries into the interior.");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "moving_boundaries", "The boundaries whose motion drives the smoothing.");
  params.addParam<std::vector<FunctionName>>(
      "velocity_functions",
      "The prescribed interface velocity of the moving boundaries, one function per mesh "
      "dimension. Mutually exclusive with velocity_variables.");
  params.addParam<std::vector<VariableName>>(
      "velocity_variables",
      "The interface velocity of the moving boundaries, read from one nodal Lagrange variable per "
      "mesh dimension. Mutually exclusive with velocity_functions.");
  params.addParam<std::vector<BoundaryName>>(
      "fixed_boundaries",
      "The boundaries pinned at a zero pseudo-displacement. Defaults to every boundary of the "
      "mesh that carries a side with no neighbor and that is not in moving_boundaries. A node on "
      "both a moving and a fixed boundary follows the moving boundary.");

  return params;
}

LaplaceSmoother::LaplaceSmoother(const InputParameters & parameters)
  : MeshSmootherBase(parameters),
    FunctionInterface(this),
    _prescribed_velocity(isParamValid("velocity_functions")),
    _sys(nullptr),
    _var_num(0)
{
  if (_prescribed_velocity && isParamValid("velocity_variables"))
    paramError("velocity_functions",
               "The interface velocity is either prescribed with velocity_functions or read from "
               "velocity_variables, but both were supplied.");
  if (!_prescribed_velocity && !isParamValid("velocity_variables"))
    mooseError("The interface velocity is required: supply either velocity_functions or "
               "velocity_variables.");

  if (getParam<std::vector<BoundaryName>>("moving_boundaries").empty())
    paramError("moving_boundaries", "At least one moving boundary is required.");

  const auto dim = _mesh.dimension();

  if (_mesh.hasSecondOrderElements())
    mooseError("The pseudo-displacement is discretized with linear Lagrange shape functions, "
               "which leaves the mid-side nodes of a second order mesh unconstrained. This "
               "smoother requires a first order mesh.");

  if (_prescribed_velocity)
  {
    const auto & names = getParam<std::vector<FunctionName>>("velocity_functions");
    if (names.size() != dim)
      paramError("velocity_functions",
                 "The mesh is ",
                 dim,
                 "-dimensional, which requires ",
                 dim,
                 " interface velocity components, but ",
                 names.size(),
                 " were supplied.");

    for (const auto & function_name : names)
      _velocity_functions.push_back(&getFunctionByName(function_name));
  }
  else
  {
    const auto & names = getParam<std::vector<VariableName>>("velocity_variables");
    if (names.size() != dim)
      paramError("velocity_variables",
                 "The mesh is ",
                 dim,
                 "-dimensional, which requires ",
                 dim,
                 " interface velocity components, but ",
                 names.size(),
                 " were supplied.");

    for (const auto & variable_name : names)
    {
      const auto & variable = _fe_problem.getVariable(/*tid=*/0,
                                                      variable_name,
                                                      Moose::VarKindType::VAR_ANY,
                                                      Moose::VarFieldType::VAR_FIELD_STANDARD);
      if (variable.feType().family != LAGRANGE)
        paramError("velocity_variables",
                   "The interface velocity component '",
                   variable_name,
                   "' is not a Lagrange variable. The moving boundary values are read at the mesh "
                   "nodes, which requires a nodal variable.");

      _velocity_variables.push_back(&variable);
    }
  }
}

void
LaplaceSmoother::setupSystem()
{
  const auto moving_ids =
      _mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("moving_boundaries"));
  _moving_boundary_ids.insert(moving_ids.begin(), moving_ids.end());

  // The default is rederived from the mesh in collectConstrainedNodes(), because the surgery can
  // change which sides have no neighbor
  if (isParamValid("fixed_boundaries"))
  {
    const auto fixed_ids =
        _mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("fixed_boundaries"));
    _fixed_boundary_ids.insert(fixed_ids.begin(), fixed_ids.end());
  }

  // libMesh stores the degrees of freedom of every system on the mesh's own Node and Elem objects,
  // so the system goes into the problem's EquationSystems rather than into a second one over the
  // same mesh, which would reset that storage for the user's systems
  _sys = &_fe_problem.es().add_system<LinearImplicitSystem>("laplace_smoother_" + name());
  _var_num = _sys->add_variable("d", FEType(FIRST, LAGRANGE));

  // The operator only changes with the mesh topology, so it is assembled here and in
  // reinitOnNewMesh() rather than on the way into every solve
  _sys->assemble_before_solve = false;

  // The Krylov solve is engine bookkeeping, so libMesh prefixes its PETSc options with the system
  // name instead of leaving it in the global namespace where a user's -pc_type would reach it
  _sys->prefix_with_name(true);

  // The pseudo-displacement is engine bookkeeping that is reset at every remesh event, so it is
  // neither output nor checkpointed
  _sys->hide_output() = true;

  _fe_problem.es().reinit();

  collectConstrainedNodes();
  assembleLaplacian();
}

std::set<BoundaryID>
LaplaceSmoother::externalBoundaryIds() const
{
  const auto & boundary_info = _mesh.getMesh().get_boundary_info();

  std::set<BoundaryID> ids;
  std::vector<BoundaryID> side_ids;
  for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
    for (const auto side : elem->side_index_range())
      if (!elem->neighbor_ptr(side))
      {
        boundary_info.boundary_ids(elem, side, side_ids);
        ids.insert(side_ids.begin(), side_ids.end());
      }

  // Only the sides of the local elements were visited, so the ranks have to agree on the result
  _communicator.set_union(ids);

  return ids;
}

void
LaplaceSmoother::collectConstrainedNodes()
{
  if (!isParamValid("fixed_boundaries"))
  {
    _fixed_boundary_ids = externalBoundaryIds();
    for (const auto id : _moving_boundary_ids)
      _fixed_boundary_ids.erase(id);
  }

  _moving_nodes.clear();
  _fixed_nodes.clear();

  // The range carries a node once per boundary it is on, so both sets are filled before the
  // overlap between them is resolved
  for (const auto & bnode : *_mesh.getBoundaryNodeRange())
  {
    const libMesh::Node * const node = bnode->_node;
    if (_moving_boundary_ids.count(bnode->_bnd_id))
      _moving_nodes.emplace(node->id(), node);
    else if (_fixed_boundary_ids.count(bnode->_bnd_id))
      _fixed_nodes.emplace(node->id(), node);
  }

  // A node where a moving boundary meets a fixed one follows the moving boundary, so that a moving
  // boundary is not pinned at the corners it shares with the pinned ones
  for (const auto & [id, _] : _moving_nodes)
    _fixed_nodes.erase(id);

  // A fixed wall that shares a corner node with a moving boundary must let its nodes slide
  // tangentially, or the corner travelling along it overtakes its stationary nodes and folds the
  // boundary on itself. Only such walls slide; every other fixed wall keeps the full pin, so a
  // problem whose moving boundary never touches a wall behaves exactly as before.
  std::set<BoundaryID> sliding_ids;
  for (const auto & bnode : *_mesh.getBoundaryNodeRange())
    if (_fixed_boundary_ids.count(bnode->_bnd_id) && _moving_nodes.count(bnode->_node->id()))
      sliding_ids.insert(bnode->_bnd_id);
  _communicator.set_union(sliding_ids);

  // Which components each fixed node is pinned along. On a sliding wall the pinned components
  // come from the side normals, evaluated with the nodes sitting at X0: a flat axis-perpendicular
  // wall pins only its normal component, an oblique one pins every component its normal points
  // along. A side of a non-sliding fixed wall pins all of them.
  _fixed_node_components.clear();
  // A component that is zero analytically comes out of the unit normal as floating-point noise, so
  // it is only taken as pointing along an axis above that noise
  constexpr Real normal_component_tol = 1e-8;
  const auto dim = _mesh.dimension();
  const auto & boundary_info = _mesh.getMesh().get_boundary_info();
  std::vector<BoundaryID> side_ids;
  for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
    for (const auto side : elem->side_index_range())
    {
      boundary_info.boundary_ids(elem, side, side_ids);
      if (std::none_of(side_ids.begin(),
                       side_ids.end(),
                       [this](const BoundaryID id) { return _fixed_boundary_ids.count(id); }))
        continue;

      const bool sliding =
          std::any_of(side_ids.begin(),
                      side_ids.end(),
                      [&sliding_ids](const BoundaryID id) { return sliding_ids.count(id); });

      const auto side_nodes = elem->nodes_on_side(side);
      Point normal(1, 0, 0);
      if (sliding)
      {
        if (dim == 2)
        {
          const Point tangent = *elem->node_ptr(side_nodes[1]) - *elem->node_ptr(side_nodes[0]);
          normal = Point(tangent(1), -tangent(0), 0);
        }
        else if (dim == 3)
          normal = (*elem->node_ptr(side_nodes[1]) - *elem->node_ptr(side_nodes[0]))
                       .cross(*elem->node_ptr(side_nodes[2]) - *elem->node_ptr(side_nodes[0]));
        normal /= normal.norm();
      }

      for (const auto local_node : side_nodes)
      {
        const auto it = _fixed_nodes.find(elem->node_ptr(local_node)->id());
        if (it == _fixed_nodes.end())
          continue;
        auto & pinned = _fixed_node_components[it->first];
        for (const auto i : make_range(dim))
          if (!sliding || std::abs(normal(i)) > normal_component_tol)
            pinned[i] = true;
      }
    }
}

void
LaplaceSmoother::assembleLaplacian()
{
  const auto dim = _mesh.dimension();
  auto & mesh = _mesh.getMesh();
  const auto & dof_map = _sys->get_dof_map();
  const auto sys_num = _sys->number();

  const auto & x0 = referenceCoordinates();

  // The operator is the one of Omega_0, but the nodes currently sit at x = X0 + d, so they go back
  // to X0 for the duration of the assembly
  for (auto & node : mesh.node_ptr_range())
  {
    const auto & reference = libmesh_map_find(x0, node->id());
    for (const auto i : make_range(Moose::dim))
      (*node)(i) = reference(i);
  }

  const FEType & fe_type = dof_map.variable_type(_var_num);
  std::unique_ptr<FEBase> fe(FEBase::build(dim, fe_type));
  QGauss qrule(dim, fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);

  const auto & JxW = fe->get_JxW();
  const auto & dphi = fe->get_dphi();

  auto & matrix = _sys->get_system_matrix();
  matrix.zero();

  DenseMatrix<Number> Ke;
  std::vector<dof_id_type> dof_indices;

  for (const auto & elem : mesh.active_local_element_ptr_range())
  {
    dof_map.dof_indices(elem, dof_indices, _var_num);
    fe->reinit(elem);

    Ke.resize(dof_indices.size(), dof_indices.size());
    for (const auto qp : index_range(JxW))
      for (const auto i : index_range(dof_indices))
        for (const auto j : index_range(dof_indices))
          Ke(i, j) += JxW[qp] * (dphi[i][qp] * dphi[j][qp]);

    matrix.add_matrix(Ke, dof_indices);
  }
  matrix.close();

  // The identity rows differ by spatial component, because a fixed wall pins only the components
  // its normal points along. The operator is kept pristine here as the base every component solve
  // restores before applying its own rows in updatePseudoDisplacement().
  _base_matrix = matrix.clone();

  _moving_rows.clear();
  for (auto & rows : _fixed_rows)
    rows.clear();
  for (const auto & [_, node] : _moving_nodes)
    if (node->processor_id() == processor_id())
      _moving_rows.push_back(node->dof_number(sys_num, _var_num, 0));
  // A fixed node with no recorded side, which can happen for a node carried by a nodeset alone,
  // defaults to the full pin
  const std::array<bool, 3> full_pin = {true, true, true};
  for (const auto & [id, node] : _fixed_nodes)
    if (node->processor_id() == processor_id())
    {
      const auto it = _fixed_node_components.find(id);
      const auto & pinned = it == _fixed_node_components.end() ? full_pin : it->second;
      for (const auto i : make_range(_mesh.dimension()))
        if (pinned[i])
          _fixed_rows[i].push_back(node->dof_number(sys_num, _var_num, 0));
    }

  // Put the nodes back exactly where the engine left them
  placeNodesAtPseudoDisplacement();
}

Real
LaplaceSmoother::interfaceVelocity(const libMesh::Node & node,
                                   const unsigned int component,
                                   const Real t) const
{
  if (_prescribed_velocity)
    return _velocity_functions[component]->value(t, node);

  const auto & variable = *_velocity_variables[component];
  const auto & system = variable.sys().system();

  return (*system.current_local_solution)(node.dof_number(system.number(), variable.number(), 0));
}

void
LaplaceSmoother::computeBoundaryValues(const Real dt, std::map<dof_id_type, Point> & values)
{
  values.clear();

  const auto dim = _mesh.dimension();
  const auto sys_num = _sys->number();
  const auto & d = pseudoDisplacement();

  // remeshingStep() runs before the executioner increments the time, so the step the mesh is being
  // moved for ends at time() + dt
  const Real t = _fe_problem.time() + dt;

  for (const auto & [id, node] : _moving_nodes)
  {
    if (node->processor_id() != processor_id())
      continue;

    // The solve is for the total displacement since the last remesh, so the moving boundary keeps
    // travelling from where it already is
    Point value = libmesh_map_find(d, id);
    for (const auto i : make_range(dim))
      value(i) += dt * interfaceVelocity(*node, i, t);

    values[node->dof_number(sys_num, _var_num, 0)] = value;
  }

  for (const auto & [_, node] : _fixed_nodes)
    if (node->processor_id() == processor_id())
      values[node->dof_number(sys_num, _var_num, 0)] = Point();
}

void
LaplaceSmoother::updatePseudoDisplacement(const Real dt)
{
  if (!_sys)
    setupSystem();

  // The boundary values are read out of the previous pseudo-displacement, so they are all computed
  // before any of it is overwritten below
  std::map<dof_id_type, Point> boundary_values;
  computeBoundaryValues(dt, boundary_values);

  auto & mesh = _mesh.getMesh();
  const auto sys_num = _sys->number();
  auto & d = pseudoDisplacement();

  // The engine applies x = X0 + d over the local and the ghosted nodes alike, so the solution has
  // to reach the nodes this rank does not own as well
  std::vector<dof_id_type> ghosted_nodes;
  std::vector<numeric_index_type> ghosted_dofs;
  for (const auto & node : mesh.node_ptr_range())
    if (node->processor_id() != processor_id())
    {
      ghosted_nodes.push_back(node->id());
      ghosted_dofs.push_back(node->dof_number(sys_num, _var_num, 0));
    }

  std::vector<Number> ghosted_values;

  // The components share the base operator but pin different rows: a fixed wall constrains only
  // the components its normal points along, so its nodes slide tangentially. The constrained rows
  // become identity rows with their columns left in place, which is what carries the prescribed
  // boundary values into the interior equations.
  for (const auto i : make_range(_mesh.dimension()))
  {
    auto & matrix = _sys->get_system_matrix();
    matrix.zero();
    matrix.add(1.0, *_base_matrix);
    matrix.close();

    std::vector<numeric_index_type> constrained_rows = _moving_rows;
    constrained_rows.insert(constrained_rows.end(), _fixed_rows[i].begin(), _fixed_rows[i].end());
    matrix.zero_rows(constrained_rows, 1.0);
    matrix.close();

    _sys->rhs->zero();
    for (const auto & [dof, value] : boundary_values)
      _sys->rhs->set(dof, value(i));
    _sys->rhs->close();

    // libMesh hands the Krylov solver a nonzero initial guess and the components share one
    // solution vector, so the previous component's field must not be left in it as that guess
    _sys->solution->zero();

    // The components pin different rows and so differ in sparsity, and clear() destroys the KSP:
    // a preconditioner that caches structure must not be handed the previous component's
    _sys->get_linear_solver()->clear();

    _sys->solve();

    // A diverged solve leaves the initial guess in the solution vector, and the values below are
    // written straight into the mesh coordinates, so it can never be allowed to pass silently
    const auto reason = _sys->get_linear_solver()->get_converged_reason();
    if (reason < 0)
      mooseError("The pseudo-displacement solve for spatial component ",
                 i,
                 " did not converge: the linear solver reported LinearConvergenceReason ",
                 static_cast<int>(reason),
                 ".");

    _sys->solution->localize(ghosted_values, ghosted_dofs);

    for (const auto & node : mesh.local_node_ptr_range())
      libmesh_map_find(d, node->id())(i) =
          (*_sys->solution)(node->dof_number(sys_num, _var_num, 0));

    for (const auto k : index_range(ghosted_nodes))
      libmesh_map_find(d, ghosted_nodes[k])(i) = ghosted_values[k];
  }
}

void
LaplaceSmoother::reset()
{
  // The elements and nodes the surgery replaces are being deleted around this call, so the node
  // pointers are dropped here and rebuilt in reinitOnNewMesh()
  _moving_nodes.clear();
  _fixed_nodes.clear();
}

void
LaplaceSmoother::reinitOnNewMesh()
{
  mooseAssert(_sys,
              "The internal system is built on the first pseudo-displacement update, which "
              "the engine performs before it can remesh.");

  // FEProblemBase::meshChanged() has already redistributed the degrees of freedom of the internal
  // system, and the engine has already snapshotted the new X0
  collectConstrainedNodes();
  assembleLaplacian();
}
