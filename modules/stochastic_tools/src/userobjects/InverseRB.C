#include "DEIMRBMapping.h"
#include "InverseRB.h"
#include "NonlinearSystemBase.h"
#include "libmesh/dense_vector.h"
#include "libmesh/elem_range.h"
#include "libmesh/implicit_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"

registerMooseObject("StochasticToolsApp", InverseRB);

InputParameters
InverseRB::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("");
  params.addRequiredParam<UserObjectName>(
      "mapping", "The name of the mapping object which provides the inverse mapping function.");
  params.addParam<dof_id_type>(
      "max_iter", 1e2, "Maximum number of iterations for the newton updates.");
  params.addParam<Real>("tolerance", 1e-6, "Convergence tolerance for the newton loop.");
  params.addParam<Real>("relaxation_factor", 1, "relaxation_factor");
  return params;
}

InverseRB::InverseRB(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    MappingInterface(this),
    _dof_map(_sys.dofMap()),
    _max_iter(getParam<dof_id_type>("max_iter")),
    _tolerance(getParam<Real>("tolerance")),
    _relax_factor(getParam<Real>("relaxation_factor")),
    _nl_sys(_fe_problem.getNonlinearSystemBase(0))
{
}

void
InverseRB::initialize()
{
  _jac_matrix = &static_cast<ImplicitSystem &>(_nl_sys.system()).get_system_matrix();
  _residual = &_nl_sys.RHS();
  _curr_sol = &*_nl_sys.currentSolution();
}

void
InverseRB::execute()
{
  // Start with an initial guess of 0 for the reduced solution
  // This will not work for a few things.
  dof_id_type reduced_size = _mapping->getReducedSize();
  DenseVector<Real> reduced_sol(reduced_size);

  // Main loop (for instance, until convergence)
  bool is_converged = false;
  unsigned int iter = 0;
  updateSolution(reduced_sol);
  while (!is_converged && iter < _max_iter)
  {
    // Calculate reduced Jacobian and residual
    auto reduced_jac = computeReducedJacobian();
    auto reduced_res = computeReducedResidual();
    reduced_res.scale(-1.0);

    // Perform linear solve (without updating the reduced solution yet)
    DenseVector<Real> reduced_sol_update;
    reduced_jac.lu_solve(reduced_res, reduced_sol_update);
    reduced_sol.add(_relax_factor, reduced_sol_update);

    // Update the full solution and check for convergence
    updateSolution(reduced_sol);
    Real res = computeResidual();

    is_converged = res <= _tolerance;

    iter++;
  }

  if (!is_converged)
  {
    // Handle non-convergence scenario
  }
}

void
InverseRB::finalize()
{
}

void
InverseRB::initialSetup()
{
  // Retrieve the mapping object. This is used to get the indices for the residual and Jacobian.
  const auto var_map = &getMapping("mapping");

  // Dynamic cast to ensure var_map is of type DEIMRBMapping. This is crucial
  // for accessing specific methods later.
  _mapping = dynamic_cast<DEIMRBMapping *>(var_map);
  mooseAssert(_mapping, "The DEIMRBMapping does not exist.");

  // Extract the indices for the residual and Jacobian matrices from the mapping.
  _residual_inds = _mapping->getResidualSelectionIndices();
  _jacobian_matrix_inds = _mapping->getJacobianSelectionIndices();

  // Create a set of dofs for the Jacobian. This set avoids duplicate entries.
  std::set<dof_id_type> jac_dofs_set;
  for (const auto & dof_pair : _jacobian_matrix_inds)
  {
    jac_dofs_set.insert(dof_pair.first);
    jac_dofs_set.insert(dof_pair.second);
  }

  std::vector<dof_id_type> jacobian_vec_inds(jac_dofs_set.begin(), jac_dofs_set.end());

  // Determine the reduced element ranges for both the Jacobian and the residual.
  // These ranges represent a subset of elements that are relevant for our calculations.
  _red_jac_elem = findReducedElemRange(jacobian_vec_inds);
  _red_res_elem = findReducedElemRange(_residual_inds);

  // Similarly, determine the reduced node ranges for both the Jacobian and the residual.
  _red_jac_node = findReducedNodeRange(jacobian_vec_inds);
  _red_res_node = findReducedNodeRange(_residual_inds);

  // Create local ranges for elements and nodes. These ranges are used to run calculations
  // on a subset of elements and nodes, improving efficiency.
  _red_jac_elem_local_range =
      createRangeFromVector<ConstElemRange, Elem, MeshBase::const_element_iterator>(_red_jac_elem);
  _red_res_elem_local_range =
      createRangeFromVector<ConstElemRange, Elem, MeshBase::const_element_iterator>(_red_res_elem);
  _red_jac_node_local_range =
      createRangeFromVector<ConstNodeRange, Node, MeshBase::const_node_iterator>(_red_jac_node);
  _red_res_node_local_range =
      createRangeFromVector<ConstNodeRange, Node, MeshBase::const_node_iterator>(_red_res_node);
}

std::vector<const Elem *>
InverseRB::findReducedElemRange(const std::vector<dof_id_type> & dofs)
{
  std::vector<const Elem *> reduced_elems;

  // Convert dofs to an unordered_set for faster lookup
  std::unordered_set<dof_id_type> dofs_set(dofs.begin(), dofs.end());

  for (const auto elem : *_fe_problem.mesh().getActiveLocalElementRange())
  {
    std::vector<dof_id_type> elem_dofs;
    _dof_map.dof_indices(elem, elem_dofs);

    // Check if any dof in elem_dofs is in dofs_set
    for (const auto & dof : elem_dofs)
    {
      if (dofs_set.find(dof) != dofs_set.end())
      {
        reduced_elems.push_back(elem);
        break; // No need to check other dofs for this element
      }
    }
  }

  return reduced_elems;
}

std::vector<const Node *>
InverseRB::findReducedNodeRange(const std::vector<dof_id_type> & dofs)
{
  std::vector<const Node *> reduced_nodes;

  // Convert dofs to an unordered_set for faster lookup
  std::unordered_set<dof_id_type> dofs_set(dofs.begin(), dofs.end());

  // Iterate over nodes in the mesh
  for (const auto node : *_fe_problem.mesh().getLocalNodeRange())
  {

    std::vector<dof_id_type> node_dofs;
    _dof_map.dof_indices(node, node_dofs);

    // Check if any dof in node_dofs is in dofs_set
    for (const auto & dof : node_dofs)
    {
      if (dofs_set.find(dof) != dofs_set.end())
      {
        reduced_nodes.push_back(node);
        break; // No need to check other dofs for this node
      }
    }
  }

  return reduced_nodes;
}

template <typename RangeType, typename ItemType, typename IteratorType>
std::unique_ptr<RangeType>
InverseRB::createRangeFromVector(const std::vector<const ItemType *> & items)
{
  // Convert vector to raw pointers for range creation
  ItemType * const * item_itr_begin = const_cast<ItemType * const *>(items.data());
  ItemType * const * item_itr_end = item_itr_begin + items.size();

  // Create iterators for the range
  const auto range_begin = IteratorType(
      item_itr_begin, item_itr_end, libMesh::Predicates::NotNull<const ItemType * const *>());
  const auto range_end = IteratorType(
      item_itr_end, item_itr_end, libMesh::Predicates::NotNull<const ItemType * const *>());

  // Create and return the unique_ptr to the range
  return std::make_unique<RangeType>(range_begin, range_end);
}

std::vector<Real>
InverseRB::getReducedJacValues(const SparseMatrix<Number> & jac)
{
  std::unordered_map<std::pair<dof_id_type, dof_id_type>, Real> jac_map;

  auto local_start = jac.row_start();
  auto local_end = jac.row_stop();

  for (const auto & index_pair : _jacobian_matrix_inds)
  {
    dof_id_type row_index = index_pair.first;
    dof_id_type col_index = index_pair.second;

    if (row_index >= local_start && row_index < local_end)
    {
      Real value = jac(row_index, col_index);
      jac_map[index_pair] = value;
    }
  }
  // Root has the correct map now
  _communicator.set_union(jac_map);
  std::vector<Real> red_jac_values;
  for (const auto & index_pair : _jacobian_matrix_inds)
    red_jac_values.push_back(jac_map.at(index_pair));

  _communicator.broadcast(red_jac_values);

  return red_jac_values;
}

std::vector<Real>
InverseRB::getReducedResValues(const NumericVector<Number> & res)
{
  std::unordered_map<dof_id_type, Real> res_map;

  auto local_start = res.first_local_index();
  auto local_end = res.last_local_index();

  for (const auto & index : _residual_inds)
  {
    if (index >= local_start && index < local_end)
    {
      Real value = res(index);
      res_map[index] = value;
    }
  }

  // Root has the correct map now
  _communicator.set_union(res_map);
  std::vector<Real> red_res_values;
  for (const auto & index : _residual_inds)
    red_res_values.push_back(res_map.at(index));

  _communicator.broadcast(red_res_values);

  return red_res_values;
}
DenseMatrix<Real>
InverseRB::computeReducedJacobian()
{
  // Compute the full Jacobian.
  // TODO: We want to use the ranges later to only compute a subset of the elements
  _fe_problem.computeJacobian(*_curr_sol, *_jac_matrix, 0);
  // Extract reduced Jacobian values and compute reduced Jacobian
  auto red_jac_values = getReducedJacValues(*_jac_matrix);
  return _mapping->compute_reduced_jac(red_jac_values);
}

DenseVector<Real>
InverseRB::computeReducedResidual()
{
  // Compute the full residual
  _fe_problem.computeResidual(*_curr_sol, *_residual, 0);
  // Extract reduced residual values and compute reduced residual
  auto red_res_values = getReducedResValues(*_residual);
  return _mapping->compute_reduced_res(red_res_values);
}

void
InverseRB::updateSolution(const DenseVector<Real> & reduced_sol)
{
  DenseVector<Real> full_solution(_curr_sol->size());
  _mapping->inverse_map("solution", reduced_sol.get_values(), full_solution);

  auto & local_sol = _nl_sys.solution();
  for (const auto & i : make_range(local_sol.first_local_index(), local_sol.last_local_index()))
    local_sol.set(i, full_solution(i));

  local_sol.close();
  _nl_sys.update();
}

Real
InverseRB::computeResidual()
{
  _fe_problem.computeResidual(*_curr_sol, *_residual, 0);
  auto error = _residual->l2_norm();
  return error;
}
