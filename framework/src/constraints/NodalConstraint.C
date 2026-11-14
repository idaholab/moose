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
#include "DisplacedProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/compare_elems_by_level.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/null_output_iterator.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"
#include "libmesh/sparse_matrix.h"

#include <algorithm>

InputParameters
NodalConstraint::validParams()
{
  InputParameters params = Constraint::validParams();
  MooseEnum formulationtype("penalty kinematic", "penalty");
  params.addParam<MooseEnum>("formulation",
                             formulationtype,
                             "Formulation used to calculate constraint - penalty or kinematic.");
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
  else
    mooseError("Formulation must be either Penalty or Kinematic");
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
NodalConstraint::ghostConnectedElems(const std::vector<dof_id_type> & node_ids)
{
  const auto elem_ids = gatherAndRetainConnectedElems(_fe_problem.mesh(), node_ids);
  for (const auto elem_id : elem_ids)
    _subproblem.addGhostedElem(elem_id);

  if (const auto displaced_problem = _fe_problem.getDisplacedProblem())
  {
    const auto displaced_elem_ids =
        gatherAndRetainConnectedElems(displaced_problem->mesh(), node_ids);
    if (displaced_elem_ids != elem_ids)
      mooseError("Reference and displaced meshes selected different primary elements");
  }
}

void
NodalConstraint::reinitConstraintNodes()
{
  // _subproblem is the displaced problem when this constraint uses the displaced mesh, which is
  // where its variables (and therefore the dof indices the assembly loops iterate over) live.
  _subproblem.reinitNodes(_primary_node_vector, _tid);
  _subproblem.reinitNodesNeighbor(_connected_nodes, _tid);

  // Derived classes index per-primary data by position in the primary node list (weights, for
  // instance), so a primary node that contributes no dof would silently shift every subsequent
  // entry. reinitNodes() drops nodes that are not semilocal or that the variable is not defined
  // on, so compare the counts. Every primary node is ghosted onto every process, so a process
  // that assembles this constraint must resolve all of them.
  const auto n_primary_dofs = _var.dofIndices().size();
  if (n_primary_dofs != _primary_node_vector.size())
    mooseError("Found ",
               n_primary_dofs,
               " primary degrees of freedom for ",
               _primary_node_vector.size(),
               " primary nodes. Nodal constraints index per-primary data by position in the "
               "primary node list, so variable '",
               _var.name(),
               "' must be defined on every primary node.");
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

  DenseMatrix<Number> Ken(primarydof.size(), secondarydof.size());
  DenseMatrix<Number> Kne(secondarydof.size(), primarydof.size());

  Ken.zero();
  Kne.zero();

  // The primary-primary and secondary-secondary couplings are both diagonal under either
  // formulation, so they are accumulated here and cached one entry at a time below. Inserting
  // them as dense blocks would instead demand sparsity for entries that are always zero, and two
  // primary nodes are only coupled in the sparsity pattern when they happen to share an element.
  std::vector<Number> Kee_diag(primarydof.size(), 0.);
  std::vector<Number> Knn_diag(secondarydof.size(), 0.);

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    for (_j = 0; _j < primarydof.size(); ++_j)
    {
      switch (_formulation)
      {
        case Moose::Penalty:
          Kee_diag[_j] += computeQpJacobian(Moose::PrimaryPrimary);
          Ken(_j, _i) += computeQpJacobian(Moose::PrimarySecondary);
          Kne(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);
          // Accumulated over the primary nodes to match the secondary residual, which sums one
          // contribution per primary node.
          Knn_diag[_i] += computeQpJacobian(Moose::SecondarySecondary);
          break;
        case Moose::Kinematic:
          Kee_diag[_j] = 0.;
          Ken(_j, _i) += jacobian(secondarydof[_i], primarydof[_j]) * _weights[_j];
          Kne(_i, _j) += -jacobian(secondarydof[_i], primarydof[_j]) / primarydof.size() +
                         computeQpJacobian(Moose::SecondaryPrimary);
          Knn_diag[_i] += -jacobian(secondarydof[_i], secondarydof[_i]) / primarydof.size() +
                          computeQpJacobian(Moose::SecondarySecondary);
          break;
      }
    }
  }
  addJacobian(_assembly, Ken, primarydof, secondarydof, _var.scalingFactor());
  addJacobian(_assembly, Kne, secondarydof, primarydof, _var_secondary.scalingFactor());

  for (const auto j : index_range(primarydof))
    addJacobianElement(_assembly, Kee_diag[j], primarydof[j], primarydof[j], _var.scalingFactor());

  for (const auto i : index_range(secondarydof))
    addJacobianElement(
        _assembly, Knn_diag[i], secondarydof[i], secondarydof[i], _var_secondary.scalingFactor());
}

void
NodalConstraint::updateConnectivity()
{
}
