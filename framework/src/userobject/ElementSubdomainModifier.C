//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSubdomainModifier.h"

InputParameters
ElementSubdomainModifier::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Modify element subdomain ID. This userobject only runs on the undisplaced mesh, and it will "
      "modify both the undisplaced and the displaced mesh.");
  params.addParam<bool>("apply_initial_conditions",
                        true,
                        "Whether to apply initial conditions on the moved nodes and elements");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

ElementSubdomainModifier::ElementSubdomainModifier(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _apply_ic(getParam<bool>("apply_initial_conditions"))
{
}

void
ElementSubdomainModifier::initialize()
{
  _moved_elems.clear();
  _moved_displaced_elems.clear();
  _moved_nodes.clear();
}

void
ElementSubdomainModifier::execute()
{
  // First, compute the desired subdomain ID for the current element.
  SubdomainID subdomain_id = computeSubdomainID();

  // If the current element's subdomain ID isn't what we want
  if (subdomain_id != std::numeric_limits<SubdomainID>::max() &&
      _current_elem->subdomain_id() != subdomain_id)
  {
    // Current element ID, used to index both the element on the displaced and undisplaced meshes.
    dof_id_type elem_id = _current_elem->id();
    // Change the element's subdomain
    Elem * elem = _mesh.elemPtr(elem_id);
    Elem * displaced_elem =
        _displaced_problem ? _displaced_problem->mesh().elemPtr(elem_id) : nullptr;

    // Save the affected nodes so that we can later update/initialize the solution
    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      _moved_nodes.insert(elem->node_id(i));

    elem->subdomain_id() = subdomain_id;
    _moved_elems.push_back(elem);
    if (displaced_elem)
    {
      displaced_elem->subdomain_id() = subdomain_id;
      _moved_displaced_elems.push_back(displaced_elem);
    }
  }
}

void
ElementSubdomainModifier::finalize()
{
  /*
    Synchronize ghost element subdomain ID
    Note: this needs to be done before updating boundary info because
    updating boundary requires the updated element subdomain ids
  */
  SyncSubdomainIds sync(_mesh.getMesh());
  Parallel::sync_dofobject_data_by_id(_mesh.getMesh().comm(),
                                      _mesh.getMesh().elements_begin(),
                                      _mesh.getMesh().elements_end(),
                                      sync);
  updateBoundaryInfo(_mesh, _moved_elems);

  // Similarly for the displaced mesh
  if (_displaced_problem)
  {
    SyncSubdomainIds sync_displaced(_displaced_problem->mesh().getMesh());
    Parallel::sync_dofobject_data_by_id(_displaced_problem->mesh().getMesh().comm(),
                                        _displaced_problem->mesh().getMesh().elements_begin(),
                                        _displaced_problem->mesh().getMesh().elements_end(),
                                        sync_displaced);
    updateBoundaryInfo(_displaced_problem->mesh(), _moved_displaced_elems);
  }

  // Reinit equation systems
  _fe_problem.meshChanged();

  // Apply initial condition for the newly moved elements and boundary nodes
  buildMovedElemsRange();
  buildMovedBndNodesRange();

  if (_apply_ic)
  {
    _fe_problem.projectInitialConditionOnCustomRange(movedElemsRange(), movedBndNodesRange());

    // Set old and older solution on the initialized dofs
    setOldAndOlderSolutionsForMovedNodes(_fe_problem.getNonlinearSystemBase());
    setOldAndOlderSolutionsForMovedNodes(_fe_problem.getAuxiliarySystem());

    if (_fe_problem.isTransient())
      _fe_problem.restoreSolutions();
  }

  // Initialize stateful material properties for the newly activated elements
  _fe_problem.initElementStatefulProps(movedElemsRange());
}

void
ElementSubdomainModifier::updateBoundaryInfo(MooseMesh & /*mesh*/,
                                             const std::vector<const Elem *> & /*moved_elems*/)
{
  return;
}

void
ElementSubdomainModifier::recordNodeIdsOnElemSide(const Elem * elem,
                                                  const unsigned short int side,
                                                  std::set<dof_id_type> & node_ids)
{
  for (unsigned int i = 0; i < elem->side_ptr(side)->n_nodes(); ++i)
    node_ids.insert(elem->side_ptr(side)->node_id(i));
}

void
ElementSubdomainModifier::buildMovedElemsRange()
{
  // Clear the object first
  _moved_elems_range.reset();

  // Make some fake element iterators defining this vector of elements
  Elem * const * elem_itr_begin = const_cast<Elem * const *>(_moved_elems.data());
  Elem * const * elem_itr_end = elem_itr_begin + _moved_elems.size();

  const auto elems_begin = MeshBase::const_element_iterator(
      elem_itr_begin, elem_itr_end, Predicates::NotNull<Elem * const *>());
  const auto elems_end = MeshBase::const_element_iterator(
      elem_itr_end, elem_itr_end, Predicates::NotNull<Elem * const *>());

  _moved_elems_range = libmesh_make_unique<ConstElemRange>(elems_begin, elems_end);
}

void
ElementSubdomainModifier::buildMovedBndNodesRange()
{
  // This is more involved than building the element range, because not all moved nodes are
  // necessarily associated with a boundary initial condition. We need to first build a set of
  // boundary nodes. Clear the object first:
  _moved_bnd_nodes_range.reset();

  // create a vector of the newly activated nodes
  std::set<const BndNode *> moved_bnd_nodes_set;
  for (auto & bnd_node : *_mesh.getBoundaryNodeRange())
  {
    dof_id_type bnd_node_id = bnd_node->_node->id();
    if (_moved_nodes.find(bnd_node_id) != _moved_nodes.end())
      moved_bnd_nodes_set.insert(bnd_node);
  }

  // Dump all the boundary nodes into a vector so that we can build a range out of it
  std::vector<const BndNode *> moved_bnd_nodes;
  moved_bnd_nodes.assign(moved_bnd_nodes_set.begin(), moved_bnd_nodes_set.end());

  // Make some fake node iterators defining this vector of nodes
  BndNode * const * bnd_node_itr_begin = const_cast<BndNode * const *>(moved_bnd_nodes.data());
  BndNode * const * bnd_node_itr_end = bnd_node_itr_begin + moved_bnd_nodes.size();

  const auto bnd_nodes_begin = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_begin, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());
  const auto bnd_nodes_end = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_end, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());

  _moved_bnd_nodes_range = libmesh_make_unique<ConstBndNodeRange>(bnd_nodes_begin, bnd_nodes_end);
}

void
ElementSubdomainModifier::setOldAndOlderSolutionsForMovedNodes(SystemBase & sys)
{
  // Don't do anything if this is a steady simulation
  if (!sys.hasSolutionState(1))
    return;

  ConstBndNodeRange & bnd_node_range = movedBndNodesRange();

  NumericVector<Number> & current_solution = *sys.system().current_local_solution;
  NumericVector<Number> & old_solution = sys.solutionOld();
  NumericVector<Number> * older_solution = sys.hasSolutionState(2) ? &sys.solutionOlder() : nullptr;

  DofMap & dof_map = sys.dofMap();

  // Get dofs for the newly added elements
  std::vector<dof_id_type> dofs;
  for (auto & bnd_node : bnd_node_range)
  {
    std::vector<dof_id_type> bnd_node_dofs;
    dof_map.dof_indices(bnd_node->_node, bnd_node_dofs);
    dofs.insert(dofs.end(), bnd_node_dofs.begin(), bnd_node_dofs.end());
  }

  // Set the old and older solution to match the IC.
  for (auto dof : dofs)
  {
    old_solution.set(dof, current_solution(dof));
    if (older_solution)
      older_solution->set(dof, current_solution(dof));
  }

  old_solution.close();
  if (older_solution)
    older_solution->close();
}
