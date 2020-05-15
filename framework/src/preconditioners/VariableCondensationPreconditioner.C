//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableCondensationPreconditioner.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseUtils.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "ComputeJacobianBlocksThread.h"
#include "MooseEnum.h"

#include "libmesh/coupling_matrix.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/mesh_base.h"
#include "libmesh/variable.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/parallel_object.h"
#include "libmesh/boundary_info.h"

#include <petscmat.h>

registerMooseObjectAliased("MooseApp", VariableCondensationPreconditioner, "VCP");

InputParameters
VariableCondensationPreconditioner::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();

  params.addClassDescription(
      "Varialble condensation preconditioner (VCP) condenses out specified variable(s) "
      "from the Jacobian matrix and produces a system of equations with less unkowns to "
      "be solved by the underlying preconditioners.");

  params.addParam<std::vector<NonlinearVariableName>>(
      "off_diag_row",
      "The off diagonal row you want to add into the matrix, it will be associated "
      "with an off diagonal column from the same position in off_diag_colum.");
  params.addParam<std::vector<NonlinearVariableName>>(
      "off_diag_column",
      "The off diagonal column you want to add into the matrix, it will be "
      "associated with an off diagonal row from the same position in "
      "off_diag_row.");
  params.addParam<std::vector<NonlinearVariableName>>(
      "coupled_groups",
      "List multiple space separated groups of comma separated variables. "
      "Off-diagonal jacobians will be generated for all pairs within a group.");
  params.addParam<bool>("full",
                        false,
                        "Set to true if you want the full set of couplings.  Simply "
                        "for convenience so you don't have to set every "
                        "off_diag_row and off_diag_column combination.");

  params.addParam<bool>(
      "is_lm_coupling_diagonal",
      false,
      "Set to true if you are sure the coupling matrix between Lagrange multiplier variable and "
      "the coupled primal variable is strict diagonal. This will speedup the linear solve. "
      "Otherwise set to false to ensure linear solve accuracy.");
  params.addParam<bool>(
      "adaptive_condensation",
      true,
      "By default VCP will check the Jacobian and only condense the rows with zero diagonals. Set "
      "to false if you want to condense out all the specified variable dofs.");
  params.addRequiredParam<std::vector<std::string>>("preconditioner", "Preconditioner type.");
  params.addRequiredParam<std::vector<std::string>>(
      "lm_variable",
      "Name of the variable(s) that is to be condensed out. Usually "
      "this will be the Lagrange multiplier variable(s).");
  params.addRequiredParam<std::vector<std::string>>(
      "primary_variable",
      "Name of the variable(s) that couples with the variable(s) specified in the `variable` "
      "block. Usually this is the primary variable that the Lagrange multiplier correspond to.");
  return params;
}

VariableCondensationPreconditioner::VariableCondensationPreconditioner(
    const InputParameters & params)
  : MoosePreconditioner(params),
    Preconditioner<Number>(MoosePreconditioner::_communicator),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _mesh(_fe_problem.mesh()),
    _dofmap(_nl.system().get_dof_map()),
    _is_lm_coupling_diagonal(getParam<bool>("is_lm_coupling_diagonal")),
    _adaptive_condensation(getParam<bool>("adaptive_condensation")),
    _n_vars(_nl.nVariables()),
    _lm_var_names(getParam<std::vector<std::string>>("lm_variable")),
    _primary_var_names(getParam<std::vector<std::string>>("primary_variable")),
    _D(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
    _M(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
    _K(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
    _dinv(nullptr),
    _J_condensed(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
    _x_hat(NumericVector<Number>::build(MoosePreconditioner::_communicator)),
    _y_hat(NumericVector<Number>::build(MoosePreconditioner::_communicator)),
    _primary_rhs_vec(NumericVector<Number>::build(MoosePreconditioner::_communicator)),
    _lm_sol_vec(NumericVector<Number>::build(MoosePreconditioner::_communicator)),
    _need_condense(true),
    _init_timer(registerTimedSection("init", 2)),
    _apply_timer(registerTimedSection("apply", 1))
{
  if (_lm_var_names.size() != _primary_var_names.size())
    paramError("coupled_variable", "coupled_variable should have the same size as the variable.");

  // get variable ids from the variable names
  for (auto var_name : _lm_var_names)
  {
    if (!_nl.system().has_variable(var_name))
      paramError("variable ", var_name, " does not exist in the system");
    unsigned int id = _nl.system().variable_number(var_name);
    _lm_var_ids.push_back(id);
  }

  // get coupled variable ids from the coupled variable names
  for (auto var_name : _primary_var_names)
  {
    if (!_nl.system().has_variable(var_name))
      paramError("coupled_variable ", var_name, " does not exist in the system");
    unsigned int id = _nl.system().variable_number(var_name);
    _primary_var_ids.push_back(id);
  }

  // PC type
  const std::vector<std::string> & pc_type = getParam<std::vector<std::string>>("preconditioner");
  if (pc_type.size() > 1)
    mooseWarning("We only use one preconditioner type in VCP, the ",
                 pc_type[0],
                 " preconditioner is utilized.");
  _pre_type = Utility::string_to_enum<PreconditionerType>(pc_type[0]);

  // The following obtains and sets the coupling matrix.
  // TODO: This part can be refactored together with what are in other classes, e.g.,
  // PhysicsBasedPreconditioner
  std::unique_ptr<CouplingMatrix> cm = libmesh_make_unique<CouplingMatrix>(_n_vars);
  bool full = getParam<bool>("full");

  if (!full)
  {
    // put 1s on diagonal
    for (unsigned int i = 0; i < _n_vars; i++)
      (*cm)(i, i) = 1;

    // off-diagonal entries from the off_diag_row and off_diag_column parameters
    std::vector<std::vector<unsigned int>> off_diag(_n_vars);
    for (unsigned int i = 0;
         i < getParam<std::vector<NonlinearVariableName>>("off_diag_row").size();
         i++)
    {
      unsigned int row =
          _nl.getVariable(0, getParam<std::vector<NonlinearVariableName>>("off_diag_row")[i])
              .number();
      unsigned int column =
          _nl.getVariable(0, getParam<std::vector<NonlinearVariableName>>("off_diag_column")[i])
              .number();
      (*cm)(row, column) = 1;
    }

    // off-diagonal entries from the coupled_groups parameters
    std::vector<NonlinearVariableName> groups =
        getParam<std::vector<NonlinearVariableName>>("coupled_groups");
    for (unsigned int i = 0; i < groups.size(); ++i)
    {
      std::vector<NonlinearVariableName> vars;
      MooseUtils::tokenize<NonlinearVariableName>(groups[i], vars, 1, ",");
      for (unsigned int j = 0; j < vars.size(); ++j)
        for (unsigned int k = j + 1; k < vars.size(); ++k)
        {
          unsigned int row = _nl.getVariable(0, vars[j]).number();
          unsigned int column = _nl.getVariable(0, vars[k]).number();
          (*cm)(row, column) = 1;
          (*cm)(column, row) = 1;
        }
    }
  }
  else
  {
    for (unsigned int i = 0; i < _n_vars; i++)
      for (unsigned int j = 0; j < _n_vars; j++)
        (*cm)(i, j) = 1;
  }

  _fe_problem.setCouplingMatrix(std::move(cm));

  _nl.attachPreconditioner(this);
}

VariableCondensationPreconditioner::~VariableCondensationPreconditioner() { this->clear(); }

void
VariableCondensationPreconditioner::getDofToCondense()
{
  // clean the containers if we want to update the dofs
  if (!_global_lm_dofs.empty())
    _global_lm_dofs.clear();
  if (!_lm_dofs.empty())
    _lm_dofs.clear();
  if (!_global_primary_dofs.empty())
    _global_primary_dofs.clear();
  if (!_primary_dofs.empty())
    _primary_dofs.clear();
  if (!_map_global_lm_primary.empty())
    _map_global_lm_primary.clear();
  if (!_map_global_primary_order.empty())
    _map_global_primary_order.clear();

  NodeRange * active_nodes = _mesh.getActiveNodeRange();

  // loop through the variable ids
  for (auto vn : index_range(_lm_var_ids))
    for (const auto & node : *active_nodes)
    {
      std::vector<dof_id_type> di;
      std::vector<dof_id_type> cp_di;
      auto var_id = _lm_var_ids[vn];
      auto cp_var_id = _primary_var_ids[vn];
      // get var and cp_var dofs associated with this node
      _dofmap.dof_indices(node, di, var_id);
      // skip when di is empty
      if (di.empty())
        continue;
      _dofmap.dof_indices(node, cp_di, cp_var_id);
      if (cp_di.size() != di.size())
        mooseError("variable and coupled variable do not have the same number of dof on node ",
                   node->id(),
                   ".");
      for (auto i : index_range(di))
      {
        // when we have adaptive condensation, skip when di does not contain any indices in
        // _zero_rows
        if (std::find(_zero_rows.begin(), _zero_rows.end(), di[i]) == _zero_rows.end() &&
            _adaptive_condensation)
          break;
        _global_lm_dofs.push_back(di[i]);
        if (_dofmap.local_index(di[i]))
          _lm_dofs.push_back(di[i]);

        // save the corresponding coupled dof indices
        _global_primary_dofs.push_back(cp_di[i]);
        if (_dofmap.local_index(cp_di[i]))
          _primary_dofs.push_back(cp_di[i]);
        _map_global_lm_primary.insert(std::make_pair(di[i], cp_di[i]));
      }
    }

  // check if we endup with none dof to condense
  if (_global_lm_dofs.empty())
  {
    _need_condense = false;
    _console << std::endl
             << "The variable(s) provided do not have a saddle-point character at this step. VCP "
                "will continue without condensing the dofs."
             << std::endl
             << std::endl;
  }
  else
    _need_condense = true;

  std::sort(_global_lm_dofs.begin(), _global_lm_dofs.end());
  std::sort(_lm_dofs.begin(), _lm_dofs.end());

  std::sort(_global_primary_dofs.begin(), _global_primary_dofs.end());
  std::sort(_primary_dofs.begin(), _primary_dofs.end());

  for (auto i : index_range(_global_lm_dofs))
  {
    auto it = _map_global_lm_primary.find(_global_lm_dofs[i]);
    mooseAssert(it != _map_global_lm_primary.end(), "Index does not exist in the map.");
    _map_global_primary_order.insert(std::make_pair(it->second, i));
  }
}

void
VariableCondensationPreconditioner::getDofColRow()
{
  // clean the containers if we want to update the dofs
  if (!_global_rows.empty())
    _global_rows.clear();
  if (!_rows.empty())
    _rows.clear();
  if (!_global_cols.empty())
    _global_cols.clear();
  if (!_cols.empty())
    _cols.clear();
  if (!_global_rows_to_idx.empty())
    _global_rows_to_idx.clear();
  if (!_rows_to_idx.empty())
    _rows_to_idx.clear();
  if (!_global_cols_to_idx.empty())
    _global_cols_to_idx.clear();
  if (!_cols_to_idx.empty())
    _cols_to_idx.clear();
  // row: all without primary variable dofs
  // col: all without lm variable dofs
  for (numeric_index_type i = 0; i < _dofmap.n_dofs(); ++i)
  {
    if (std::find(_global_primary_dofs.begin(), _global_primary_dofs.end(), i) !=
        _global_primary_dofs.end())
      continue;

    _global_rows.push_back(i);
    _global_rows_to_idx.insert(std::make_pair(i, _global_rows.size() - 1));
    if (_dofmap.local_index(i))
    {
      _rows.push_back(i);
      _rows_to_idx.insert(std::make_pair(i, _global_rows_to_idx[i]));
    }

    // ensure the lm and primary correspondance, so that the condensed Jacobian has non-zero
    // diagonal if the dof corresponds to the lm variable, then find the corresponding primary
    // variable dof and add to _global_cols
    if (_map_global_lm_primary.find(i) != _map_global_lm_primary.end())
    {
      auto primary_idx = _map_global_lm_primary[i];
      _global_cols.push_back(primary_idx);
      _global_cols_to_idx.insert(std::make_pair(primary_idx, _global_cols.size() - 1));

      if (_dofmap.local_index(primary_idx))
      {
        _cols.push_back(primary_idx);
        _cols_to_idx.insert(std::make_pair(primary_idx, _global_cols_to_idx[primary_idx]));
      }
    }
    else // if the dof does not correspond to the lm nor primary varialble, just add to _global_cols
    {
      _global_cols.push_back(i);
      _global_cols_to_idx.insert(std::make_pair(i, _global_cols.size() - 1));

      if (_dofmap.local_index(i))
      {
        _cols.push_back(i);
        _cols_to_idx.insert(std::make_pair(i, _global_cols_to_idx[i]));
      }
    }
  }
}

void
VariableCondensationPreconditioner::init()
{
  TIME_SECTION(_init_timer);

  if (!_preconditioner)
    _preconditioner =
        Preconditioner<Number>::build_preconditioner(MoosePreconditioner::_communicator);

  _is_initialized = true;
}

void
VariableCondensationPreconditioner::condenseSystem()
{

  // extract _M from the original matrix
  _matrix->create_submatrix(*_M, _rows, _lm_dofs);

  // get the row associated with the coupled primary variable
  _K->init(_global_primary_dofs.size(), _global_cols.size(), _primary_dofs.size(), _cols.size());
  MatSetOption(_K->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
  // here the _global_cols may not be sorted
  _matrix->create_submatrix_nosort(*_K, _global_primary_dofs, _global_cols);

  _matrix->create_submatrix(*_D, _primary_dofs, _lm_dofs);

  PetscErrorCode ierr = 0;
  // clean dinv
  if (_dinv)
  {
    ierr = MatDestroy(&_dinv);
    LIBMESH_CHKERR(ierr);
    _dinv = nullptr;
  }

  // Compute inverse of D
  if (_is_lm_coupling_diagonal)
  {
    // when _D is strictly diagonal, we only need to compute the reciprocal number of the diagonal
    // entries
    computeDInverseDiag(_dinv);
  }
  else
  {
    // for general cases when _D is not necessarily strict diagonal, we compute the inverse of _D
    // using LU
    computeDInverse(_dinv);
  }

  Mat MdinvK;
  // calculate MdinvK
  ierr = MatMatMatMult(_M->mat(), _dinv, _K->mat(), MAT_INITIAL_MATRIX, PETSC_DEFAULT, &MdinvK);
  LIBMESH_CHKERR(ierr);
  PetscMatrix<Number> MDinv_K(MdinvK, MoosePreconditioner::_communicator);

  // Preallocate memory for _J_condensed
  // memory info is obtained from _matrix and MDinv_K
  // indices are from _global_rows, _global_cols
  preallocateCondensedJacobian(
      *_J_condensed, *_matrix, _rows, _cols, _global_rows, _global_cols, MDinv_K);

  // Extract unchanged parts from _matrix and add changed parts (MDinv_K) to _J_condensed
  computeCondensedJacobian(*_J_condensed, *_matrix, _global_rows, MDinv_K);

  // Destroy MdinvK here otherwise we will have memory leak
  ierr = MatDestroy(&MdinvK);
  LIBMESH_CHKERR(ierr);
}

void
VariableCondensationPreconditioner::computeCondensedJacobian(
    PetscMatrix<Number> & condensed_mat,
    SparseMatrix<Number> & original_mat,
    const std::vector<numeric_index_type> & grows,
    PetscMatrix<Number> & block_mat)
{
  auto pc_original_mat = cast_ptr<PetscMatrix<Number> *>(&original_mat);

  PetscErrorCode ierr = 0;

  // obtain entries from the original matrix
  PetscInt pc_ncols = 0, block_ncols = 0;
  const PetscInt *pc_cols, *block_cols;
  const PetscScalar *pc_vals, *block_vals;

  // containers for the data
  std::vector<PetscInt> sub_cols;
  std::vector<PetscScalar> sub_vals;

  for (auto i : index_range(grows))
  {
    PetscInt sub_rid[] = {static_cast<PetscInt>(i)};
    PetscInt rid = static_cast<PetscInt>(grows[i]);
    if (grows[i] >= pc_original_mat->row_start() && grows[i] < pc_original_mat->row_stop())
    {
      // get one row of data from the original matrix
      ierr = MatGetRow(pc_original_mat->mat(), rid, &pc_ncols, &pc_cols, &pc_vals);
      LIBMESH_CHKERR(ierr);
      // get corresponding row of data from the block matrix
      ierr = MatGetRow(block_mat.mat(), i, &block_ncols, &block_cols, &block_vals);
      LIBMESH_CHKERR(ierr);
      // extract data from certain cols, subtract the value from the block mat, and save the indices
      // and entries sub_cols and sub_vals
      // First, save the submatrix col index and value as a map
      std::map<unsigned int, PetscScalar> pc_col_map;
      for (unsigned int pc_idx = 0; pc_idx < static_cast<unsigned int>(pc_ncols); pc_idx++)
      {
        // save only if the col exists in the condensed matrix
        if (_global_cols_to_idx.find(static_cast<unsigned int>(pc_cols[pc_idx])) !=
            _global_cols_to_idx.end())
          pc_col_map.insert(std::make_pair(
              _global_cols_to_idx[static_cast<unsigned int>(pc_cols[pc_idx])], pc_vals[pc_idx]));
      }
      // Second, check the block cols and calculate new entries for the condensed system
      for (unsigned int block_idx = 0; block_idx < static_cast<unsigned int>(block_ncols);
           block_idx++)
      {
        unsigned int block_col = static_cast<unsigned int>(block_cols[block_idx]);
        PetscScalar block_val = block_vals[block_idx];
        // if the block mat has nonzero at the same column, subtract value
        // otherwise, create a new key and save the negative value from the block matrix
        if (pc_col_map.find(block_col) != pc_col_map.end())
          pc_col_map[block_col] -= block_val;
        else
          pc_col_map[block_col] = -block_val;
      }

      // Third, save keys in the sub_cols and values in the sub_vals
      for (std::map<unsigned int, PetscScalar>::iterator it = pc_col_map.begin();
           it != pc_col_map.end();
           ++it)
      {
        sub_cols.push_back(static_cast<PetscInt>(it->first));
        sub_vals.push_back(it->second);
      }

      // Then, set values
      ierr = MatSetValues(condensed_mat.mat(),
                          1,
                          sub_rid,
                          static_cast<PetscInt>(sub_vals.size()),
                          sub_cols.data(),
                          sub_vals.data(),
                          INSERT_VALUES);
      LIBMESH_CHKERR(ierr);
      ierr = MatRestoreRow(pc_original_mat->mat(), rid, &pc_ncols, &pc_cols, &pc_vals);
      LIBMESH_CHKERR(ierr);
      ierr = MatRestoreRow(block_mat.mat(), i, &block_ncols, &block_cols, &block_vals);
      LIBMESH_CHKERR(ierr);
      // clear data for this row
      sub_cols.clear();
      sub_vals.clear();
    }
  }
  condensed_mat.close();
}

void
VariableCondensationPreconditioner::preallocateCondensedJacobian(
    PetscMatrix<Number> & condensed_mat,
    SparseMatrix<Number> & original_mat,
    const std::vector<numeric_index_type> & rows,
    const std::vector<numeric_index_type> & cols,
    const std::vector<numeric_index_type> & grows,
    const std::vector<numeric_index_type> & gcols,
    PetscMatrix<Number> & block_mat)
{
  auto pc_original_mat = cast_ptr<PetscMatrix<Number> *>(&original_mat);

  // quantities from the original matrix and the block matrix
  PetscInt ncols = 0, block_ncols = 0;
  const PetscInt * col_vals;
  const PetscInt * block_col_vals;
  const PetscScalar * vals;
  const PetscScalar * block_vals;

  std::vector<PetscInt> block_cols_to_org; // stores the nonzero column indices of the block
                                           // matrix w.r.t original matrix
  std::vector<PetscInt>
      merged_cols; // stores the nonzero column indices estimate of the condensed matrix

  // number of nonzeros in each row of the DIAGONAL and OFF-DIAGONAL portion of the local
  // condensed matrix
  std::vector<numeric_index_type> n_nz, n_oz;

  PetscErrorCode ierr = 0;

  // Get number of nonzeros from original_mat and block_mat for each row
  for (auto row_id : _rows)
  {
    // get number of non-zeros in the original matrix
    ierr =
        MatGetRow(pc_original_mat->mat(), static_cast<PetscInt>(row_id), &ncols, &col_vals, &vals);
    LIBMESH_CHKERR(ierr);

    // get number of non-zeros in the block matrix
    numeric_index_type block_row_id; // row id in the block matrix

    if (_global_rows_to_idx.find(row_id) != _global_rows_to_idx.end())
      block_row_id = _global_rows_to_idx[row_id];
    else
      mooseError("DoF ", row_id, " does not exist in the rows of condensed_mat");

    ierr = MatGetRow(block_mat.mat(),
                     static_cast<PetscInt>(block_row_id),
                     &block_ncols,
                     &block_col_vals,
                     &block_vals);
    LIBMESH_CHKERR(ierr);

    // make sure the block index is transformed in terms of the original mat
    block_cols_to_org.clear();
    for (PetscInt i = 0; i < block_ncols; ++i)
    {
      auto idx = gcols[static_cast<numeric_index_type>(block_col_vals[i])];
      block_cols_to_org.push_back(static_cast<PetscInt>(idx));
    }

    // Now store nonzero column indices for the condensed Jacobian
    // merge `col_vals` and `block_cols_to_org` and save the common indices in `merged_cols`.
    mergeArrays(col_vals, block_cols_to_org.data(), ncols, block_ncols, merged_cols);

    ierr = MatRestoreRow(block_mat.mat(),
                         static_cast<PetscInt>(block_row_id),
                         &block_ncols,
                         &block_col_vals,
                         &block_vals);
    LIBMESH_CHKERR(ierr);

    ierr = MatRestoreRow(
        pc_original_mat->mat(), static_cast<PetscInt>(row_id), &ncols, &col_vals, &vals);
    LIBMESH_CHKERR(ierr);

    // Count the nnz for DIAGONAL and OFF-DIAGONAL parts
    unsigned int row_n_nz = 0, row_n_oz = 0;
    for (auto merged_col : merged_cols)
    {
      // find corresponding index in the block mat and skip the cols that do not exist in the
      // condensed system
      if (_global_cols_to_idx.find(merged_col) == _global_cols_to_idx.end())
        continue;

      numeric_index_type col_idx = _global_cols_to_idx[merged_col];
      // find the corresponding row index
      numeric_index_type row_idx = grows[col_idx];
      // check whether the index is local;
      // yes - DIAGONAL, no - OFF-DIAGONAL
      if (_rows_to_idx.find(row_idx) != _rows_to_idx.end())
        row_n_nz++;
      else
        row_n_oz++;
    }

    n_nz.push_back(cast_int<numeric_index_type>(row_n_nz));
    n_oz.push_back(cast_int<numeric_index_type>(row_n_oz));
  }
  // Then initialize and allocate memory for the condensed system matrix
  condensed_mat.init(grows.size(), gcols.size(), rows.size(), cols.size(), n_nz, n_oz);
}

void
VariableCondensationPreconditioner::mergeArrays(const PetscInt * a,
                                                const PetscInt * b,
                                                const PetscInt & na,
                                                const PetscInt & nb,
                                                std::vector<PetscInt> & c)
{
  c.clear();

  // use map to store unique elements.
  std::map<PetscInt, bool> mp;

  // Inserting values to a map.
  for (int i = 0; i < na; i++)
    mp[a[i]] = true;

  for (int i = 0; i < nb; i++)
    mp[b[i]] = true;

  // Save the merged values to c, if only the value also exist in gcols
  for (auto i : mp)
    c.push_back(i.first);
}

void
VariableCondensationPreconditioner::setup()
{
  if (_adaptive_condensation)
    findZeroDiagonals(*_matrix, _zero_rows);

  // save dofs that are to be condensed out
  getDofToCondense();

  // solve the condensed system only when needed, otherwise solve the original system
  if (_need_condense)
  {
    // get condensed dofs for rows and cols
    getDofColRow();

    condenseSystem();

    // make sure diagonal entries are not empty
    for (auto i = _J_condensed->row_start(); i < _J_condensed->row_stop(); ++i)
      _J_condensed->add(i, i, 0.0);
    _J_condensed->close();

    _preconditioner->set_matrix(*_J_condensed);
  }
  else
    _preconditioner->set_matrix(*_matrix);

  _preconditioner->set_type(_pre_type);
  _preconditioner->init();
}

void
VariableCondensationPreconditioner::apply(const NumericVector<Number> & y,
                                          NumericVector<Number> & x)
{
  TIME_SECTION(_apply_timer);

  if (_need_condense)
  {
    getCondensedXY(y, x);

    _preconditioner->apply(*_y_hat, *_x_hat);

    computeCondensedVariables();

    getFullSolution(y, x);
  }
  else
  {
    _preconditioner->apply(y, x);
  }
}

void
VariableCondensationPreconditioner::getCondensedXY(const NumericVector<Number> & y,
                                                   NumericVector<Number> & x)
{
  Mat mdinv;
  PetscErrorCode ierr = 0;
  // calculate mdinv
  ierr = MatMatMult(_M->mat(), _dinv, MAT_INITIAL_MATRIX, PETSC_DEFAULT, &mdinv);
  LIBMESH_CHKERR(ierr);
  PetscMatrix<Number> MDinv(mdinv, MoosePreconditioner::_communicator);

  _x_hat->init(_J_condensed->n(), _J_condensed->local_n(), false, PARALLEL);
  _y_hat->init(_J_condensed->m(), _J_condensed->local_m(), false, PARALLEL);

  x.create_subvector(*_x_hat, _global_cols);
  y.create_subvector(*_y_hat, _global_rows);

  _primary_rhs_vec->init(MDinv.n(), MDinv.local_n(), false, PARALLEL);

  std::unique_ptr<NumericVector<Number>> mdinv_primary_rhs(
      NumericVector<Number>::build(MoosePreconditioner::_communicator));
  mdinv_primary_rhs->init(MDinv.m(), MDinv.local_m(), false, PARALLEL);

  // get _primary_rhs_vec from the original y
  y.create_subvector(*_primary_rhs_vec, _global_primary_dofs);

  MDinv.vector_mult(*mdinv_primary_rhs, *_primary_rhs_vec);
  mdinv_primary_rhs->close();

  (*_y_hat) -= (*mdinv_primary_rhs);

  _y_hat->close();
  _x_hat->close();

  ierr = MatDestroy(&mdinv);
  LIBMESH_CHKERR(ierr);
}

void
VariableCondensationPreconditioner::computeCondensedVariables()
{
  // clear the vector
  _lm_sol_vec->clear();

  PetscMatrix<Number> Dinv(_dinv, MoosePreconditioner::_communicator);

  _lm_sol_vec->init(_D->m(), _D->local_m(), false, PARALLEL);

  std::unique_ptr<NumericVector<Number>> K_xhat(
      NumericVector<Number>::build(MoosePreconditioner::_communicator));
  K_xhat->init(_K->m(), _K->local_m(), false, PARALLEL);
  _K->vector_mult(*K_xhat, *_x_hat);
  K_xhat->close();

  (*_primary_rhs_vec) -= (*K_xhat);
  _primary_rhs_vec->close();
  Dinv.vector_mult(*_lm_sol_vec, *_primary_rhs_vec);
  _lm_sol_vec->close();
}

void
VariableCondensationPreconditioner::getFullSolution(const NumericVector<Number> & /*y*/,
                                                    NumericVector<Number> & x)
{
  std::vector<numeric_index_type> dof_indices;
  std::vector<Number> vals;

  // save values and indices from _x_hat and _lm_sol_vec
  for (auto i = _x_hat->first_local_index(); i < _x_hat->last_local_index(); ++i)
  {
    dof_indices.push_back(_global_cols[i]);
    vals.push_back((*_x_hat)(i));
  }

  for (auto i = _lm_sol_vec->first_local_index(); i < _lm_sol_vec->last_local_index(); ++i)
  {
    dof_indices.push_back(_global_lm_dofs[i]);
    vals.push_back((*_lm_sol_vec)(i));
  }

  x.insert(vals.data(), dof_indices);
  x.close();
}

void
VariableCondensationPreconditioner::findZeroDiagonals(SparseMatrix<Number> & mat,
                                                      std::vector<numeric_index_type> & indices)
{
  indices.clear();
  IS zerodiags, zerodiags_all;
  PetscErrorCode ierr;
  const PetscInt * petsc_idx;
  PetscInt nrows;
  // make sure we have a petsc matrix
  PetscMatrix<Number> * petsc_mat = cast_ptr<PetscMatrix<Number> *>(&mat);
  ierr = MatFindZeroDiagonals(petsc_mat->mat(), &zerodiags);
  LIBMESH_CHKERR(ierr);
  // synchronize all indices
  ierr = ISAllGather(zerodiags, &zerodiags_all);
  LIBMESH_CHKERR(ierr);
  ierr = ISGetIndices(zerodiags_all, &petsc_idx);
  LIBMESH_CHKERR(ierr);
  ierr = ISGetSize(zerodiags_all, &nrows);
  LIBMESH_CHKERR(ierr);

  for (PetscInt i = 0; i < nrows; ++i)
    indices.push_back(static_cast<numeric_index_type>(petsc_idx[i]));

  ISRestoreIndices(zerodiags_all, &petsc_idx);
  LIBMESH_CHKERR(ierr);
  ierr = ISDestroy(&zerodiags);
  LIBMESH_CHKERR(ierr);
  ierr = ISDestroy(&zerodiags_all);
  LIBMESH_CHKERR(ierr);
}

void
VariableCondensationPreconditioner::clear()
{
  PetscErrorCode ierr;
  if (_dinv != nullptr)
  {
    ierr = MatDestroy(&_dinv);
    LIBMESH_CHKERR(ierr);
  }
}

void
VariableCondensationPreconditioner::computeDInverse(Mat & dinv)
{
  PetscErrorCode ierr;
  Mat F, I, dinv_dense;
  IS perm, iperm;
  MatFactorInfo info;

  ierr = MatCreateDense(PETSC_COMM_WORLD,
                        static_cast<PetscInt>(_D->local_n()),
                        static_cast<PetscInt>(_D->local_m()),
                        static_cast<PetscInt>(_D->n()),
                        static_cast<PetscInt>(_D->m()),
                        NULL,
                        &dinv_dense);
  LIBMESH_CHKERR(ierr);

  // Create an identity matrix as the right-hand-side
  ierr = MatCreateDense(PETSC_COMM_WORLD,
                        static_cast<PetscInt>(_D->local_m()),
                        static_cast<PetscInt>(_D->local_m()),
                        static_cast<PetscInt>(_D->m()),
                        static_cast<PetscInt>(_D->m()),
                        NULL,
                        &I);
  LIBMESH_CHKERR(ierr);

  for (unsigned int i = 0; i < _D->m(); ++i)
  {
    ierr = MatSetValue(I, static_cast<PetscInt>(i), static_cast<PetscInt>(i), 1.0, INSERT_VALUES);
    LIBMESH_CHKERR(ierr);
  }

  ierr = MatAssemblyBegin(I, MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);
  ierr = MatAssemblyEnd(I, MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);

  // Factorize D
  ierr = MatGetOrdering(_D->mat(), MATORDERINGND, &perm, &iperm);
  LIBMESH_CHKERR(ierr);

  ierr = MatFactorInfoInitialize(&info);
  LIBMESH_CHKERR(ierr);

  ierr = MatGetFactor(_D->mat(), MATSOLVERSUPERLU_DIST, MAT_FACTOR_LU, &F);
  LIBMESH_CHKERR(ierr);

  ierr = MatLUFactorSymbolic(F, _D->mat(), perm, iperm, &info);
  LIBMESH_CHKERR(ierr);

  ierr = MatLUFactorNumeric(F, _D->mat(), &info);
  LIBMESH_CHKERR(ierr);

  // Solve for Dinv
  ierr = MatMatSolve(F, I, dinv_dense);
  LIBMESH_CHKERR(ierr);

  ierr = MatAssemblyBegin(dinv_dense, MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);
  ierr = MatAssemblyEnd(dinv_dense, MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);

  // copy value to dinv
  ierr = MatConvert(dinv_dense, MATAIJ, MAT_INITIAL_MATRIX, &dinv);
  LIBMESH_CHKERR(ierr);

  ierr = MatDestroy(&dinv_dense);
  LIBMESH_CHKERR(ierr);

  ierr = MatDestroy(&I);
  LIBMESH_CHKERR(ierr);
  ierr = MatDestroy(&F);
  LIBMESH_CHKERR(ierr);
  ierr = ISDestroy(&perm);
  LIBMESH_CHKERR(ierr);
  ierr = ISDestroy(&iperm);
  LIBMESH_CHKERR(ierr);
}

void
VariableCondensationPreconditioner::computeDInverseDiag(Mat & dinv)
{
  PetscErrorCode ierr;
  auto diag_D = NumericVector<Number>::build(MoosePreconditioner::_communicator);
  // Initialize dinv
  ierr = MatCreateAIJ(PETSC_COMM_WORLD,
                      static_cast<PetscInt>(_D->local_n()),
                      static_cast<PetscInt>(_D->local_m()),
                      static_cast<PetscInt>(_D->n()),
                      static_cast<PetscInt>(_D->m()),
                      1,
                      NULL,
                      0,
                      NULL,
                      &dinv);
  LIBMESH_CHKERR(ierr);
  // Allocate storage
  diag_D->init(_D->m(), _D->local_m(), false, PARALLEL);
  // Fill entries
  for (numeric_index_type i = _D->row_start(); i < _D->row_stop(); ++i)
  {
    auto it = _map_global_primary_order.find(_global_primary_dofs[i]);
    mooseAssert(it != _map_global_primary_order.end(), "Index does not exist in the map.");
    diag_D->set(it->second, (*_D)(i, it->second));
  }

  for (numeric_index_type i = _D->row_start(); i < _D->row_stop(); ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual((*diag_D)(i), 0.0))
      mooseError("Trying to compute reciprocal of 0.");
    ierr = MatSetValue(dinv,
                       static_cast<PetscInt>(i),
                       static_cast<PetscInt>(_map_global_primary_order.at(_global_primary_dofs[i])),
                       static_cast<PetscScalar>(1.0 / (*diag_D)(i)),
                       INSERT_VALUES);
    LIBMESH_CHKERR(ierr);
  }

  ierr = MatAssemblyBegin(dinv, MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);
  ierr = MatAssemblyEnd(dinv, MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);
}
