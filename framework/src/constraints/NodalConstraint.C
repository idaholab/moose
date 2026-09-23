//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "NodalBCBase.h"
#include "NonlinearSystemBase.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/compare_elems_by_level.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/dof_map.h"
#include "libmesh/null_output_iterator.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"
#include "libmesh/sparse_matrix.h"

#include <algorithm>

InputParameters
NodalConstraint::validParams()
{
  InputParameters params = Constraint::validParams();
  MooseEnum formulationtype("penalty kinematic rows", "penalty");
  params.addParam<MooseEnum>(
      "formulation",
      formulationtype,
      "Formulation used to calculate constraint - penalty, kinematic or rows. With 'rows' the "
      "constraint assembles no residual and no Jacobian: it hands one degree of freedom "
      "constraint row per secondary node to the DofMap, which enforces the constraint exactly and "
      "needs no penalty.");
  params.addParam<NonlinearVariableName>("variable_secondary",
                                         "The name of the variable for the secondary nodes, if it "
                                         "is different from the primary nodes' variable");
  return params;
}

NodalConstraint::NodalConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, true),
    NeighborMooseVariableInterface<Real>(
        this, true, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _var_secondary(_sys.getFieldVariable<Real>(
        _tid,
        isParamValid("variable_secondary")
            ? parameters.get<NonlinearVariableName>("variable_secondary")
            : parameters.get<NonlinearVariableName>("variable"))),
    _u_secondary(_var_secondary.dofValuesNeighbor()),
    _u_primary(_var.dofValues())
{
  addMooseVariableDependency(&_var);
  addMooseVariableDependency(&_var_secondary);

  MooseEnum temp_formulation = getParam<MooseEnum>("formulation");
  if (temp_formulation == "penalty")
    _formulation = Moose::Penalty;
  else if (temp_formulation == "kinematic")
    _formulation = Moose::Kinematic;
  else if (temp_formulation == "rows")
    _formulation = Moose::Rows;
  else
    mooseError("Formulation must be penalty, kinematic or rows");
}

void
NodalConstraint::checkPenaltyParam() const
{
  if (_formulation != Moose::Rows && !isParamValid("penalty"))
    paramError("penalty",
               "A penalty is required with formulation '",
               getParam<MooseEnum>("formulation"),
               "'. It may only be omitted with formulation 'rows', which enforces the constraint "
               "with degree of freedom constraint rows instead of a penalty term.");
}

std::vector<dof_id_type>
NodalConstraint::gatherAndRetainConnectedElems(MooseMesh & mesh,
                                               const std::vector<dof_id_type> & node_ids)
{
  const auto & node_to_elem_map = mesh.nodeToElemMap();
  auto * const distributed_mesh = dynamic_cast<libMesh::DistributedMesh *>(&mesh.getMesh());

  // local reference to the retained elements for this mesh, so we don't have to look it up in the
  // map every time
  auto & retained_elems = _retained_elems[&mesh];

  // Elements connected to these nodes may already be remote on a distributed mesh, so gather
  // gather one locally available connected element for each node before rebuilding the connectivity
  // map.
  if (distributed_mesh)
  {
    // Mesh adaptation may delete elements retained by a previous invocation. Remove their raw
    // pointers from DistributedMesh before replacing them with the current connected elements.
    distributed_mesh->clear_extra_ghost_elems(retained_elems);
    retained_elems.clear();

    std::set<Elem *, libMesh::CompareElemIdsByLevel> elems_to_ghost;
    std::set<Node *> nodes_to_ghost;

    // Loop over each node, and find one element connected to it.
    for (const auto node_id : node_ids)
    {
      const auto node_to_elem_pair = node_to_elem_map.find(node_id);
#ifndef NDEBUG
      // Debugging check should be per node (inside the node loop)
      bool someone_found_elem = false;
#endif

      if (node_to_elem_pair != node_to_elem_map.end())
        for (const auto elem_id : node_to_elem_pair->second)
          if (auto * const elem = mesh.queryElemPtr(elem_id))
          {
            elems_to_ghost.insert(elem);
            for (const auto n : make_range(elem->n_nodes()))
              nodes_to_ghost.insert(elem->node_ptr(n));
#ifndef NDEBUG
            someone_found_elem = true;
#endif
            break; // Only need one element to retain the node
          }
#ifndef NDEBUG
      // gather through all processors to make sure at least one processor found an element for this
      // node
      mesh.getMesh().comm().max(someone_found_elem);
      mooseAssert(someone_found_elem || node_ids.empty(), "Missing entry in node to elem map");
#endif
    }

    // Send nodes first since elements need them.
    mesh.getMesh().comm().allgather_packed_range(&mesh.getMesh(),
                                                 nodes_to_ghost.begin(),
                                                 nodes_to_ghost.end(),
                                                 libMesh::null_output_iterator<Node>());
    mesh.getMesh().comm().allgather_packed_range(&mesh.getMesh(),
                                                 elems_to_ghost.begin(),
                                                 elems_to_ghost.end(),
                                                 libMesh::null_output_iterator<Elem>());

    // Rebuild the node-to-element map after gathering the remote mesh entities.
    mesh.update();
  }

  // After rebuilding connectivity, select one canonical element ID per node.
  std::vector<dof_id_type> elem_ids;
  for (const auto node_id : node_ids)
  {
    // Reacquire the iterator after mesh.update().
    const auto node_to_elem_pair = node_to_elem_map.find(node_id);
    if (node_to_elem_pair == node_to_elem_map.end() || node_to_elem_pair->second.empty())
      mooseError("Couldn't find any elements connected to primary node");

    const auto elem_id =
        node_to_elem_pair->second.front(); // Just need one element to retain the node, like above,
                                           // just need one element to be ghosted
    elem_ids.push_back(elem_id);

    // Keep gathered elements when libMesh later deletes unneeded remote elements.
    if (distributed_mesh)
    {
      auto * const elem = mesh.elemPtr(elem_id);
      distributed_mesh->add_extra_ghost_elem(elem);
      retained_elems.insert(elem);
    }
  }

  // We only need one element per node.
  mooseAssert(node_ids.size() == elem_ids.size(),
              "Mismatch between number of primary nodes and connected elements");

  return elem_ids;
}

void
NodalConstraint::reinitConstraintNodes()
{
  // _subproblem is the displaced problem when this constraint uses the displaced mesh, which is
  // where its variables (and therefore the dof indices the assembly loops iterate over) live.
  _subproblem.reinitNodes(_primary_node_vector, _tid);
  _subproblem.reinitNodesNeighbor(_connected_nodes, _tid);
}

void
NodalConstraint::computeResidual(const NumericVector<Number> & residual)
{
  if ((_weights.size() == 0) && (_primary_node_vector.size() == 1))
    _weights.push_back(1.0);

  std::vector<dof_id_type> primarydof = _var.dofIndices();
  std::vector<dof_id_type> secondarydof = _var_secondary.dofIndicesNeighbor();

  DenseVector<Number> re(primarydof.size());
  DenseVector<Number> neighbor_re(secondarydof.size());

  re.zero();
  neighbor_re.zero();

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    for (_j = 0; _j < primarydof.size(); ++_j)
    {
      switch (_formulation)
      {
        case Moose::Rows:
          mooseError("The rows formulation is enforced by the DofMap, so the enforcement loop "
                     "never asks this constraint for a residual");
          break;
        case Moose::Penalty:
          re(_j) += computeQpResidual(Moose::Primary) * _var.scalingFactor();
          neighbor_re(_i) += computeQpResidual(Moose::Secondary) * _var_secondary.scalingFactor();
          break;
        case Moose::Kinematic:
          // Transfer the current residual of the secondary node to the primary nodes
          Real res = residual(secondarydof[_i]);
          re(_j) += res * _weights[_j];
          neighbor_re(_i) +=
              -res / _primary_node_vector.size() + computeQpResidual(Moose::Secondary);
          break;
      }
    }
  }
  // We've already applied scaling
  if (!primarydof.empty())
    addResiduals(_assembly, re, primarydof, /*scaling_factor=*/1);
  if (!secondarydof.empty())
    addResiduals(_assembly, neighbor_re, secondarydof, /*scaling_factor=*/1);
}

void
NodalConstraint::computeJacobian(const SparseMatrix<Number> & jacobian)
{
  if ((_weights.size() == 0) && (_primary_node_vector.size() == 1))
    _weights.push_back(1.0);

  // Calculate the dense-block Jacobian entries
  std::vector<dof_id_type> secondarydof = _var_secondary.dofIndicesNeighbor();
  std::vector<dof_id_type> primarydof = _var.dofIndices();

  DenseMatrix<Number> Kee(primarydof.size(), primarydof.size());
  DenseMatrix<Number> Ken(primarydof.size(), secondarydof.size());
  DenseMatrix<Number> Kne(secondarydof.size(), primarydof.size());

  Kee.zero();
  Ken.zero();
  Kne.zero();

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    for (_j = 0; _j < primarydof.size(); ++_j)
    {
      switch (_formulation)
      {
        case Moose::Rows:
          mooseError("The rows formulation is enforced by the DofMap, so the enforcement loop "
                     "never asks this constraint for a Jacobian");
          break;
        case Moose::Penalty:
          Kee(_j, _j) += computeQpJacobian(Moose::PrimaryPrimary);
          Ken(_j, _i) += computeQpJacobian(Moose::PrimarySecondary);
          Kne(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);
          break;
        case Moose::Kinematic:
          Kee(_j, _j) = 0.;
          Ken(_j, _i) += jacobian(secondarydof[_i], primarydof[_j]) * _weights[_j];
          Kne(_i, _j) += -jacobian(secondarydof[_i], primarydof[_j]) / primarydof.size() +
                         computeQpJacobian(Moose::SecondaryPrimary);
          break;
      }
    }
  }
  addJacobian(_assembly, Kee, primarydof, primarydof, _var.scalingFactor());
  addJacobian(_assembly, Ken, primarydof, secondarydof, _var.scalingFactor());
  addJacobian(_assembly, Kne, secondarydof, primarydof, _var_secondary.scalingFactor());

  // Calculate and cache the diagonal secondary-secondary entries
  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    Number value = 0.0;
    switch (_formulation)
    {
      case Moose::Rows:
        mooseError("The rows formulation is enforced by the DofMap, so the enforcement loop never "
                   "asks this constraint for a Jacobian");
        break;
      case Moose::Penalty:
        value = computeQpJacobian(Moose::SecondarySecondary);
        break;
      case Moose::Kinematic:
        value = -jacobian(secondarydof[_i], secondarydof[_i]) / primarydof.size() +
                computeQpJacobian(Moose::SecondarySecondary);
        break;
    }
    addJacobianElement(
        _assembly, value, secondarydof[_i], secondarydof[_i], _var_secondary.scalingFactor());
  }
}

void
NodalConstraint::addConstraintRows(libMesh::DofMap & dof_map) const
{
  addTieRows(dof_map, _connected_nodes);
}

std::vector<dof_id_type>
NodalConstraint::ownedBoundaryNodes(const BoundaryName & boundary_name) const
{
  std::vector<dof_id_type> node_ids;
  for (const auto nid : _mesh.getNodeList(_mesh.getBoundaryID(boundary_name)))
  {
    const Node * const node = _mesh.queryNodePtr(nid);
    if (node && node->processor_id() == _subproblem.processor_id())
      node_ids.push_back(nid);
  }

  return node_ids;
}

void
NodalConstraint::addTieRows(libMesh::DofMap & dof_map,
                            const std::vector<dof_id_type> & secondary_nodes) const
{
  // The single primary node of an equal value tie carries a unit weight, the same default
  // computeResidual() applies. _weights cannot be filled in here because this method is const, and
  // it is called before the first residual evaluation anyway
  std::vector<Real> weights = _weights;
  if (weights.empty() && _primary_node_vector.size() == 1)
    weights.push_back(1.0);

  // The rows are always built on the reference mesh and in the reference system, whatever
  // 'use_displaced_mesh' says. libMesh rebuilds the constraints of the reference system while it
  // initializes it, before the displaced copy of the system has any degrees of freedom, so the
  // nodes of the displaced mesh carry no variable group data to ask for a dof number with. The two
  // systems share their degree of freedom numbering, so the rows this builds are the ones both
  // DofMaps need. This is what keeps 'use_displaced_mesh = true' legal here: the coefficients of a
  // nodal tie are parameters, not geometry
  MooseMesh & mesh = _fe_problem.mesh();
  NonlinearSystemBase & nl_sys = referenceSystem(_var_secondary.name());
  MooseVariable & var = nl_sys.getFieldVariable<Real>(_tid, _var.name());
  MooseVariable & var_secondary = nl_sys.getFieldVariable<Real>(_tid, _var_secondary.name());
  const auto sys_num = nl_sys.number();

  // A secondary node a nodal boundary condition already pins keeps its boundary condition, which
  // is what the penalty and kinematic formulations do
  const auto pinned_nodes = nodesPinnedByNodalBCs(var_secondary.name());

  // This method is collective: libMesh calls it on every rank each time it rebuilds the
  // constraints of the system
  CollectiveError error(*this);

  // A weight per primary node is needed wherever a row is emitted, and the primary nodes a rank
  // holds are the ones it found on its copy of the mesh, so this can only be checked here
  if (weights.size() != _primary_node_vector.size())
  {
    error.record("This rank holds " + std::to_string(_primary_node_vector.size()) +
                 " of the primary nodes but " + std::to_string(weights.size()) +
                 " weights. Every rank that constrains a secondary node needs all the primary "
                 "nodes and their weights, so either provide as many weights as primary nodes or "
                 "use a replicated mesh.");

    // Every rank reaches raise() exactly once, here or at the end, so the gather still matches
    error.raise();
    return;
  }

  // The dof of \p var at \p node, or invalid_id when the variable is not defined there
  auto node_dof = [&error, sys_num](const MooseVariable & var, const Node & node) -> dof_id_type
  {
    if (node.n_comp(sys_num, var.number()) == 0)
    {
      error.record("The variable '" + var.name() + "' has no degree of freedom at node " +
                   std::to_string(node.id()) + ", so this constraint cannot tie it there.");
      return libMesh::DofObject::invalid_id;
    }
    return node.dof_number(sys_num, var.number(), 0);
  };

  for (const auto secondary_id : secondary_nodes)
  {
    // The ranks that have this node add its row, and they all add the same one
    const Node * const secondary_node = mesh.queryNodePtr(secondary_id);
    if (!secondary_node)
      continue;

    // A node that is its own primary already holds the value the tie would give it, and a row
    // constraining a dof to itself is not a constraint
    if (std::find(_primary_node_vector.begin(), _primary_node_vector.end(), secondary_id) !=
        _primary_node_vector.end())
      continue;

    // A nodal boundary condition already pins this dof, and it wins, as it does under the penalty
    // and kinematic formulations
    if (pinned_nodes.count(secondary_id))
      continue;

    libMesh::DofConstraintRow row;
    bool row_is_complete = true;
    for (const auto j : index_range(_primary_node_vector))
    {
      const Node * const primary_node = mesh.queryNodePtr(_primary_node_vector[j]);
      if (!primary_node)
      {
        error.record("The primary node " + std::to_string(_primary_node_vector[j]) +
                     " is not present on the rank that holds the secondary node " +
                     std::to_string(secondary_id) +
                     ". This constraint builds its rows from node ids alone, so every primary "
                     "node must be present wherever a secondary node is; use a replicated mesh.");
        row_is_complete = false;
        break;
      }

      const auto primary_dof = node_dof(var, *primary_node);
      if (primary_dof == libMesh::DofObject::invalid_id)
      {
        row_is_complete = false;
        break;
      }

      // A node repeated in the primary list contributes the sum of its weights
      row[primary_dof] += weights[j];
    }

    if (!row_is_complete)
      break;

    const auto secondary_dof = node_dof(var_secondary, *secondary_node);
    if (secondary_dof == libMesh::DofObject::invalid_id)
      break;

    // Forbid overwriting so that a secondary dof libMesh already constrains, a hanging node for
    // instance, errors out instead of silently losing one of the two constraints
    dof_map.add_constraint_row(secondary_dof, row, /*forbid_constraint_overwrite=*/true);
  }

  error.raise();
}

void
NodalConstraint::updateConnectivity()
{
}
