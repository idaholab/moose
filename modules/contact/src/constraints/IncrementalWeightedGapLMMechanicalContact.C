//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncrementalWeightedGapLMMechanicalContact.h"
#include "SystemBase.h"
#include "MooseVariableBase.h"
#include "MooseMesh.h"
#include "libmesh/mesh_base.h"
#include "libmesh/system.h"
#include "libmesh/node.h"
#include "libmesh/petsc_matrix.h"
#include <petscmat.h>

registerMooseObject("ContactApp", IncrementalWeightedGapLMMechanicalContact);

InputParameters
IncrementalWeightedGapLMMechanicalContact::validParams()
{
  InputParameters params = ComputeWeightedGapLMMechanicalContact::validParams();
  params.addClassDescription(
      "Useful for mortar mechanical contact zero-penetration condition in dynamics contexts.");
  return params;
}

IncrementalWeightedGapLMMechanicalContact::IncrementalWeightedGapLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeWeightedGapLMMechanicalContact(assignVarsInParams(parameters)),
    _D(_sys.system().add_vector("mortar_D", false, PARALLEL)),
    _M(_sys.system().add_matrix("mortar_M")),
    _disp_x_var(*getVar("disp_x", 0)),
    _disp_y_var(*getVar("disp_y", 0)),
    _disp_z_var(_has_disp_z ? getVar("disp_z", 0) : nullptr),
    _use_vertices(_disp_x_var.feType().order == FIRST),
    _n_disp(_mesh.dimension()),
    _delta_d(NumericVector<Number>::build(this->comm())),
    _sub_D(NumericVector<Number>::build(this->comm()))
{
  if (_disp_x_var.feType().order != _var->feType().order)
    mooseError("This dynamics method only works for equal order discretization of the "
               "displacements and LM");

  if (_n_disp == 3 && !_disp_z_var)
    mooseError("For a mesh dimension of 3, we should have a z displacement variable.");

  auto check_num = [this](auto & var) {
    if (var.sys().number() != 0)
      mooseError("Why are you solving mortar mechanical contact with displacements that aren't in "
                 "the nonlinear system?");
  };

  check_num(_disp_x_var);
  check_num(_disp_y_var);
  if (_disp_z_var)
    check_num(*_disp_z_var);
}

void
IncrementalWeightedGapLMMechanicalContact::computeResidual(const Moose::MortarType mortar_type)
{
  if (mortar_type != Moose::MortarType::Lower)
    return;

  ComputeWeightedGapLMMechanicalContact::computeResidual(mortar_type);

  const auto & x_dof_indices_secondary = _disp_x_var.dofIndices();
  const auto & x_dof_indices_primary = _disp_x_var.dofIndicesNeighbor();

  // Ok we have the dof indices but this will include interior degrees of freedom as well. We cannot
  // keep those interior degrees of freedom because we are going to be indexing the Lagrange
  // Multiplier test function, and if we keep the interior dofs we will end up indexing out of
  // bounds
  const Elem * const secondary_ip = _lower_secondary_elem->interior_parent();
  const Elem * const primary_ip = _lower_primary_elem->interior_parent();
  mooseAssert(secondary_ip, "This should be non-null");
  mooseAssert(primary_ip, "This should be non-null");

  const auto ncl =
      _use_vertices ? _lower_secondary_elem->n_vertices() : _lower_secondary_elem->n_nodes();
  const auto ncm =
      _use_vertices ? _lower_primary_elem->n_vertices() : _lower_primary_elem->n_nodes();
  _secondary_lower_to_ip.resize(ncl);
  _primary_lower_to_ip.resize(ncm);

  mooseAssert(ncl == _test.size(), "These should be the same");

  for (const auto i : make_range(ncl))
  {
    const Node * const nd = _lower_secondary_elem->node_ptr(i);
    _secondary_lower_to_ip[i] = secondary_ip->get_node_index(nd);
  }

  for (const auto i : make_range(ncm))
  {
    const Node * const nd = _lower_primary_elem->node_ptr(i);
    _primary_lower_to_ip[i] = primary_ip->get_node_index(nd);
  }

  // Resize on DenseVector/DenseMatrix also zeroes them
  _local_D.resize(ncl);
  _local_M.resize(ncl, ncm);
  for (const auto qp : make_range(_qrule_msm->n_points()))
    for (const auto i : make_range(ncl))
    {
      _local_D(i) += _test_secondary[_secondary_lower_to_ip[i]][qp] * _JxW_msm[qp] * _coord[qp];
      for (const auto j : make_range(ncm))
        _local_M(i, j) +=
            _test[i][qp] * _test_primary[_primary_lower_to_ip[j]][qp] * _JxW_msm[qp] * _coord[qp];
    }

  const auto & y_dof_indices_secondary = _disp_y_var.dofIndices();
  const std::vector<dof_id_type> * const z_dof_indices_secondary =
      _disp_z_var ? &_disp_z_var->dofIndices() : nullptr;
  const auto & y_dof_indices_primary = _disp_y_var.dofIndicesNeighbor();
  const std::vector<dof_id_type> * const z_dof_indices_primary =
      _disp_z_var ? &_disp_z_var->dofIndices() : nullptr;
  for (const auto i : make_range(ncl))
  {
    const auto d_val = _local_D(i);
    _D.add(x_dof_indices_secondary[_secondary_lower_to_ip[i]], d_val);
    _D.add(y_dof_indices_secondary[_secondary_lower_to_ip[i]], d_val);
    if (_disp_z_var)
      _D.add((*z_dof_indices_secondary)[_secondary_lower_to_ip[i]], d_val);

    for (const auto j : make_range(ncm))
    {
      const auto m_val = _local_M(i, j);
      _M.add(x_dof_indices_secondary[_secondary_lower_to_ip[i]],
             x_dof_indices_primary[_primary_lower_to_ip[j]],
             m_val);
      _M.add(y_dof_indices_secondary[_secondary_lower_to_ip[i]],
             y_dof_indices_primary[_primary_lower_to_ip[j]],
             m_val);
      if (_disp_z_var)
        _M.add((*z_dof_indices_secondary)[_secondary_lower_to_ip[i]],
               (*z_dof_indices_primary)[_primary_lower_to_ip[j]],
               m_val);
    }
  }
}

void
IncrementalWeightedGapLMMechanicalContact::residualSetup()
{
  _D.zero();
  _M.zero();
  if (!feProblem().errorOnJacobianNonzeroReallocation())
    MatSetOption(
        static_cast<PetscMatrix<Number> &>(_M).mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
  ComputeWeightedGapLMMechanicalContact::residualSetup();
}

void
IncrementalWeightedGapLMMechanicalContact::initialSetup()
{
  const auto & secondary_node_list = _mesh.getNodeList(_secondary_id);

  std::vector<std::pair<dof_id_type, dof_id_type>> dof_and_node_ids;
  // Try to minimize the number of times we allocate
  dof_and_node_ids.reserve(_n_disp * secondary_node_list.size());

  const auto & lm_mesh = _mesh.getMesh();

  for (const auto node_id : secondary_node_list)
  {
    const Node * const node = lm_mesh.node_ptr(node_id);
    mooseAssert(node, "Should be non-null");
    if (node->processor_id() != this->processor_id())
      continue;

    dof_and_node_ids.push_back(
        std::make_pair(node->dof_number(0, _disp_x_var.number(), 0), node->id()));
    dof_and_node_ids.push_back(
        std::make_pair(node->dof_number(0, _disp_y_var.number(), 0), node->id()));
    if (_disp_z_var)
      dof_and_node_ids.push_back(
          std::make_pair(node->dof_number(0, _disp_z_var->number(), 0), node->id()));
  }

  const auto & primary_node_list = _mesh.getNodeList(_primary_id);

  // Try to minimize the number of times we allocate
  _cols.reserve(_n_disp * primary_node_list.size());

  for (const auto node_id : primary_node_list)
  {
    const Node * const node = lm_mesh.node_ptr(node_id);
    mooseAssert(node, "Should be non-null");
    if (node->processor_id() != this->processor_id())
      continue;

    _cols.push_back(node->dof_number(0, _disp_x_var.number(), 0));
    _cols.push_back(node->dof_number(0, _disp_y_var.number(), 0));
    if (_disp_z_var)
      _cols.push_back(node->dof_number(0, _disp_z_var->number(), 0));
  }

  dof_and_node_ids.shrink_to_fit();
  std::sort(dof_and_node_ids.begin(), dof_and_node_ids.end());
  _cols.shrink_to_fit();
  std::sort(_cols.begin(), _cols.end());

  _rows.resize(dof_and_node_ids.size());

  unsigned int disp_num = 0;
  for (const auto i : index_range(dof_and_node_ids))
  {
    const auto & pr = dof_and_node_ids[i];
    const auto global_dof = pr.first;
    const auto node_id = pr.second;
    _rows[i] = global_dof;
    _node_id_to_mortar_disp_indices[node_id][disp_num++] = i;
    disp_num %= _n_disp;
  }
}

void
IncrementalWeightedGapLMMechanicalContact::buildIncrements()
{
  const auto & u = *_sys.currentSolution();
  const auto & u_old = _sys.solutionOld();

  _D.close();
  _M.close();

  auto sub_M = SparseMatrix<Number>::build(_M.comm());
  auto d_secondary_old = NumericVector<Number>::build(u_old.comm());
  auto d_primary = NumericVector<Number>::build(u.comm());
  auto d_primary_old = NumericVector<Number>::build(u_old.comm());

  _D.create_subvector(*_sub_D, _rows);
  _M.create_submatrix(*sub_M, _rows, _cols);
  u.create_subvector(*_delta_d, _rows);
  u_old.create_subvector(*d_secondary_old, _rows);
  u.create_subvector(*d_primary, _cols);
  u_old.create_subvector(*d_primary_old, _cols);

  // Convert into increment vectors
  _delta_d->close();
  d_primary->close();
  (*_delta_d) -= *d_secondary_old;
  (*d_primary) -= *d_primary_old;

  // Compute inverse of D
  _sub_D->close();
  auto D_inv = _sub_D->clone();
  D_inv->reciprocal();

  // Mat representation of D_inv
  auto D_inv_mat = SparseMatrix<Number>::build(_M.comm());
  const auto m = D_inv->size();
  const auto m_l = D_inv->local_size();
  D_inv_mat->init(m, m, m_l, m_l, 1, 0);
  for (const auto i : make_range(D_inv->first_local_index(), D_inv->last_local_index()))
    D_inv_mat->set(i, i, (*D_inv)(i));
  D_inv_mat->close();

  // Form Mhat
  sub_M->close();
  auto Mhat = SparseMatrix<Number>::build(_M.comm());
  D_inv_mat->matrix_matrix_mult(*sub_M, *Mhat, /*reuse=*/false);

  // Form Mhat * delta_d_M product
  auto work_vec = NumericVector<Number>::build(_sub_D->comm());
  work_vec->init(m, m_l, false, PARALLEL);
  Mhat->vector_mult(*work_vec, *d_primary);

  // finish off increment computation. Hartmann equation 31
  (*_delta_d) -= *work_vec;
  _delta_d->close();
}

void
IncrementalWeightedGapLMMechanicalContact::post()
{
  buildIncrements();

  ComputeWeightedGapLMMechanicalContact::post();
}

void
IncrementalWeightedGapLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  buildIncrements();

  ComputeWeightedGapLMMechanicalContact::incorrectEdgeDroppingPost(inactive_lm_nodes);
}

void
IncrementalWeightedGapLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
  const Node * const node = static_cast<const Node *>(dof);

  // weighted gap
  const auto & weighted_gap = *_weighted_gap_ptr;
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;

  // Equation 32, Hartmann
  const auto & disp_local_mortar_indices =
      libmesh_map_find(_node_id_to_mortar_disp_indices, node->id());
  const auto & normal = amg().getNodalNormal(node);
  ADReal increment = 0;
  for (const auto i : make_range(_n_disp))
  {
    const auto current_disp_local_index = disp_local_mortar_indices[i];
    ADReal current_disp_delta = (*_delta_d)(current_disp_local_index);
    // The derivative with respect to the secondary is straightforward, but the derivative with
    // respect to the primary is a little more complicated... it involves matrix multiplication in
    // Hartmann equation 31. We'll seed the secondary for now and see whether that's good enough for
    // PJFNK
    Moose::derivInsert(current_disp_delta.derivatives(), _rows[current_disp_local_index], 1.);
    increment += normal(i) * (*_sub_D)(current_disp_local_index)*current_disp_delta;
  }

  // LM
  const auto dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  ADReal lm_value = (*_sys.currentSolution())(dof_index);
  Moose::derivInsert(lm_value.derivatives(), dof_index, 1.);

  // finally, form the NCP
  const ADReal dof_residual = std::min(lm_value, (weighted_gap - increment) * c);

  if (_subproblem.currentlyComputingJacobian())
    _assembly.processDerivatives(dof_residual, dof_index, _matrix_tags);
  else
    _assembly.cacheResidual(dof_index, dof_residual.value(), _vector_tags);
}
