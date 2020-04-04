//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPatchRecovery.h"
#include "SwapBackSentinel.h"

InputParameters
NodalPatchRecovery::validParams()
{
  InputParameters params = AuxKernel::validParams();
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH");
  params.addParam<MooseEnum>("patch_polynomial_order",
                             orders,
                             "Polynomial order used in least squares fitting of material property "
                             "over the local patch of elements connected to a given node");
  params.addParamNamesToGroup("patch_polynomial_order", "Advanced");

  // TODO make this class work with relationship manager
  // params.registerRelationshipManagers("ElementSideNeighborLayers");
  // params.addPrivateParam<unsigned int short>("element_side_neighbor_layers", 2);

  return params;
}

NodalPatchRecovery::NodalPatchRecovery(const InputParameters & parameters)
  : AuxKernel(parameters),
    _patch_polynomial_order(
        isParamValid("patch_polynomial_order")
            ? static_cast<unsigned int>(getParam<MooseEnum>("patch_polynomial_order"))
            : static_cast<unsigned int>(_var.order())),
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _multi_index(multiIndex(_mesh.dimension(), _patch_polynomial_order))
{
  // if the patch polynomial order is lower than the variable order,
  // it is very likely that the patch recovery is not used at its max accuracy
  if (_patch_polynomial_order < static_cast<unsigned int>(_var.order()))
    mooseWarning("Specified 'patch_polynomial_order' is lower than the AuxVariable's order");

  // TODO remove the manual ghosting once relationship manager is working correctly
  // no need to ghost if this aux is elemental
  if (isNodal())
  {
    MeshBase & meshhelper = _mesh.getMesh();
    meshhelper.allow_renumbering(false);
    for (const auto & elem :
         as_range(meshhelper.semilocal_elements_begin(), meshhelper.semilocal_elements_end()))
      _fe_problem.addGhostedElem(elem->id());
  }
}

std::vector<std::vector<unsigned int>>
NodalPatchRecovery::nChooseK(unsigned int N, unsigned int K)
{
  std::vector<std::vector<unsigned int>> n_choose_k;
  std::vector<unsigned int> row;
  std::string bitmask(K, 1); // K leading 1's
  bitmask.resize(N, 0);      // N-K trailing 0's

  do
  {
    row.clear();
    row.push_back(0);
    for (unsigned int i = 0; i < N; ++i) // [0..N-1] integers
      if (bitmask[i])
        row.push_back(i + 1);
    row.push_back(N + 1);
    n_choose_k.push_back(row);
  } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

  return n_choose_k;
}

std::vector<std::vector<unsigned int>>
NodalPatchRecovery::multiIndex(unsigned int dim, unsigned int order)
{
  // first row all zero
  std::vector<std::vector<unsigned int>> multi_index;
  std::vector<std::vector<unsigned int>> n_choose_k;
  std::vector<unsigned int> row(dim, 0);
  multi_index.push_back(row);

  if (dim == 1)
    for (unsigned int q = 1; q <= order; q++)
    {
      row[0] = q;
      multi_index.push_back(row);
    }
  else
    for (unsigned int q = 1; q <= order; q++)
    {
      n_choose_k = nChooseK(dim + q - 1, dim - 1);
      for (unsigned int r = 0; r < n_choose_k.size(); r++)
      {
        row.clear();
        for (unsigned int c = 1; c < n_choose_k[0].size(); c++)
          row.push_back(n_choose_k[r][c] - n_choose_k[r][c - 1] - 1);
        multi_index.push_back(row);
      }
    }

  return multi_index;
}

void
NodalPatchRecovery::computePVector(Point q_point)
{
  _P.resize(_multi_index.size());
  Real polynomial;
  for (unsigned int r = 0; r < _multi_index.size(); r++)
  {
    polynomial = 1.0;
    for (unsigned int c = 0; c < _multi_index[0].size(); c++)
      for (unsigned int p = 0; p < _multi_index[r][c]; p++)
        polynomial *= q_point(c);
    _P(r) = polynomial;
  }
}

void
NodalPatchRecovery::accumulateAMatrix()
{
  DenseMatrix<Number> PxP;
  PxP.resize(_multi_index.size(), _multi_index.size());
  PxP.outer_product(_P, _P);
  _A.add(1.0, PxP);
}

void
NodalPatchRecovery::accumulateBVector(Real val)
{
  _B.add(val, _P);
}

void
NodalPatchRecovery::reinitPatch()
{
  // clean and resize P, A, B, coef
  _P.resize(_multi_index.size());
  _A.resize(_multi_index.size(), _multi_index.size());
  _B.resize(_multi_index.size());
  _coef.resize(_multi_index.size());

  // activate dependent material properties
  std::set<unsigned int> needed_mat_props;
  const std::set<unsigned int> & mp_deps = getMatPropDependencies();
  needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
}

void
NodalPatchRecovery::compute()
{
  // Fall back on standard procedure for element variables
  if (!isNodal())
  {
    AuxKernel::compute();
    return;
  }

  // Limit current use of NodalPatchRecovery to a single processor
  if (_communicator.size() > 1)
    mooseError("The nodal patch recovery option, which calculates the Zienkiewicz-Zhu patch "
               "recovery for nodal variables (family = LAGRANGE), is not currently implemented for "
               "parallel runs. Run in serial if you must use the nodal patch capability");

  // Use Zienkiewicz-Zhu patch recovery for nodal variables
  reinitPatch();

  // get node-to-conneted-elem map
  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map = _mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != node_to_elem_map.end(), "Missing entry in node to elem map");
  std::vector<dof_id_type> elem_ids = node_to_elem_pair->second;

  // consider the case for corner node
  if (elem_ids.size() == 1)
  {
    dof_id_type elem_id = elem_ids[0];
    for (auto & n : _mesh.elemPtr(elem_id)->node_ref_range())
    {
      node_to_elem_pair = node_to_elem_map.find(n.id());
      std::vector<dof_id_type> elem_ids_candidate = node_to_elem_pair->second;
      if (elem_ids_candidate.size() > elem_ids.size())
        elem_ids = elem_ids_candidate;
    }
  }

  // before we go, check if we have enough sample points for solving the least square fitting
  if (_q_point.size() * elem_ids.size() < _multi_index.size())
    mooseError("There are not enough sample points to recover the nodal value, try reducing the "
               "polynomial order or using a higher-order quadrature scheme.");

  // general treatment for side nodes and internal nodes
  for (auto elem_id : elem_ids)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);

    if (!elem->is_semilocal(_mesh.processor_id()))
      mooseError("skipped non local elem!");

    _fe_problem.prepare(elem, _tid);
    _fe_problem.reinitElem(elem, _tid);

    // Set up Sentinel class so that, even if reinitMaterials() throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);
    _fe_problem.reinitMaterials(elem->subdomain_id(), _tid);

    for (_qp = 0; _qp < _q_point.size(); _qp++)
    {
      computePVector(_q_point[_qp]);
      accumulateAMatrix();
      accumulateBVector(computeValue());
    }
  }

  _fe_problem.clearActiveMaterialProperties(_tid);

  // solve for the coefficients of least square fitting
  // as we are on the physical coordinates, A is not always SPD
  _A.svd_solve(_B, _coef);

  // compute fitted nodal value
  computePVector(*_current_node);
  Real nodal_value = _P.dot(_coef);

  // set nodal value
  _fe_problem.reinitNode(_current_node, _tid);
  dof_id_type dof = _var.nodalDofIndex();
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _solution.set(dof, nodal_value);
  }
  _var.setNodalValue(nodal_value);
}
