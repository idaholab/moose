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
    _D_inv(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
    _MDinv(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
    _u2c_rows(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
    _J_condensed(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
    _x_hat(NumericVector<Number>::build(MoosePreconditioner::_communicator)),
    _y_hat(NumericVector<Number>::build(MoosePreconditioner::_communicator)),
    _r2c(NumericVector<Number>::build(MoosePreconditioner::_communicator)),
    _lambda(NumericVector<Number>::build(MoosePreconditioner::_communicator)),
    _save_dofs(false),
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
  if (!_glm.empty())
    _glm.clear();
  if (!_lm.empty())
    _lm.clear();
  if (!_gu2c.empty())
    _gu2c.clear();
  if (!_u2c.empty())
    _u2c.clear();
  if (!_map_glm_gu2c.empty())
    _map_glm_gu2c.clear();
  if (!_map_gu2c_glm.empty())
    _map_gu2c_glm.clear();
  if (!_map_gu2c_to_order.empty())
    _map_gu2c_to_order.clear();

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
        _glm.push_back(di[i]);
        if (_dofmap.local_index(di[i]))
          _lm.push_back(di[i]);

        // save the corresponding coupled dof indices
        _gu2c.push_back(cp_di[i]);
        if (_dofmap.local_index(cp_di[i]))
          _u2c.push_back(cp_di[i]);
        _map_glm_gu2c.insert(std::make_pair(di[i], cp_di[i]));
        _map_gu2c_glm.insert(std::make_pair(cp_di[i], di[i]));
      }
    }

  // check if we endup with none dof to condense
  if (_glm.empty())
  {
    _need_condense = false;
#ifdef DEBUG
    mooseWarning("The variable provided do not have a saddle-point character. VCP will "
                 "continue without condensing the dofs.");
#endif
    return;
  }
  else
    _need_condense = true;

  std::sort(_glm.begin(), _glm.end());
  std::sort(_lm.begin(), _lm.end());

  std::sort(_gu2c.begin(), _gu2c.end());
  std::sort(_u2c.begin(), _u2c.end());

  for (auto i : index_range(_glm))
    _map_gu2c_to_order.insert(std::make_pair(_map_glm_gu2c.at(_glm[i]), i));
}

void
VariableCondensationPreconditioner::getDofColRow()
{
  // clean the containers if we want to update the dofs
  if (!_grows.empty())
    _grows.clear();
  if (!_rows.empty())
    _rows.clear();
  if (!_gcols.empty())
    _gcols.clear();
  if (!_cols.empty())
    _cols.clear();
  // row: all without u2c
  // col: all without lm
  for (numeric_index_type i = 0; i < _dofmap.n_dofs(); ++i)
  {
    if (std::find(_gu2c.begin(), _gu2c.end(), i) != _gu2c.end())
      continue;

    _grows.push_back(i);
    if (_dofmap.local_index(i))
      _rows.push_back(i);

    // ensure the lm and u2c correspondance, so that the condensed Jacobian has non-zero diagonal
    // if the dof corresponds to the lm variable, then find the corresponding u2c dof and add to
    // _gcol
    if (_map_glm_gu2c.find(i) != _map_glm_gu2c.end())
    {
      auto u2c_idx = _map_glm_gu2c.at(i);
      _gcols.push_back(u2c_idx);

      if (_dofmap.local_index(u2c_idx))
        _cols.push_back(u2c_idx);
    }
    else // if the dof does not correspond to the lm nor u2c varialble, just add to _gcols
    {
      _gcols.push_back(i);

      if (_dofmap.local_index(i))
        _cols.push_back(i);
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
  _matrix->create_submatrix(*_M, _rows, _lm);

  // get the row associated with u2c
  _u2c_rows->init(_gu2c.size(), _gcols.size(), _u2c.size(), _cols.size());
  MatSetOption(_u2c_rows->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
  _matrix->create_submatrix_nosort(*_u2c_rows, _gu2c, _gcols);

  _matrix->create_submatrix(*_D, _u2c, _lm);

  _D_inv->init(_D->n(), _D->m(), _D->local_n(), _D->local_m());
  MatSetOption(_D_inv->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
  if (_is_lm_coupling_diagonal)
  {
    // _D should be strictly diagonal if dual_mortar approach is utilized
    // so we only need to compute the reciprocal number of the diagonal entries
    computeDInverseDiag();
  }
  else
  {
    // for general cases when we condense LMs, D is not necessarily diagonal
    // compute the inverse of D using MatMatSolve()
    computeDInverse();
  }
  _D_inv->close();

#ifdef DEBUG
  // check if we get the inversion correctly
  PetscMatrix<Number> I(MoosePreconditioner::_communicator);
  _D->matrix_matrix_mult(*_D_inv, I);
  for (unsigned int i = I.row_start(); i < I.row_stop(); ++i)
    if (!MooseUtils::absoluteFuzzyEqual(I(i, i), 1.0))
      mooseError("Inverse of D is wrong.");
#endif
  // compute MDinv=_M*_Dinv
  _M->matrix_matrix_mult(*_D_inv, *_MDinv);

  // initialize _J_condensed
  _J_condensed->init(_grows.size(), _gcols.size(), _rows.size(), _cols.size());
  MatSetOption(_J_condensed->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
  _matrix->create_submatrix_nosort(*_J_condensed, _grows, _gcols);

  // compute changed parts: MDinv* [K2ci, K2cc]
  std::unique_ptr<PetscMatrix<Number>> MDinv_u2c_rows(
      libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator));

  _MDinv->matrix_matrix_mult(*_u2c_rows, *MDinv_u2c_rows);

  // add changed parts to _J_condensed
  // original system row_id: _grows
  // original system col_id: _gcols
  std::map<numeric_index_type, numeric_index_type> row_id_mp, col_id_mp;

  for (auto it : index_range(_grows))
  {
    numeric_index_type lid = static_cast<numeric_index_type>(it);
    auto it_row = find(_grows.begin(), _grows.end(), _grows[it]);
    if (it_row != _grows.end())
    {
      numeric_index_type gid = std::distance(_grows.begin(), it_row);
      if (lid >= MDinv_u2c_rows->row_start() && lid < MDinv_u2c_rows->row_stop())
        row_id_mp.insert(std::make_pair(lid, gid));
    }
    else
      mooseError("DOF ", _grows[it], " does not exist in the rows of the condensed system");
  }

  // map for cols
  for (auto it : index_range(_gcols))
  {
    numeric_index_type lid = static_cast<numeric_index_type>(it);
    col_id_mp.insert(std::make_pair(lid, lid));
  }

  // one-step add-matrix
  _J_condensed->add_sparse_matrix(*MDinv_u2c_rows, row_id_mp, col_id_mp, -1.0);
  _J_condensed->close();
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
  // create a copy of y
  std::unique_ptr<NumericVector<Number>> y_copy(
      NumericVector<Number>::build(MoosePreconditioner::_communicator));
  y_copy = y.clone();

  _x_hat->init(_J_condensed->n(), _J_condensed->local_n(), false, PARALLEL);
  _y_hat->init(_J_condensed->m(), _J_condensed->local_m(), false, PARALLEL);

  x.create_subvector(*_x_hat, _gcols);

  _r2c->init(_MDinv->n(), _MDinv->local_n(), false, PARALLEL);

  std::unique_ptr<NumericVector<Number>> mdinv_r2c(
      NumericVector<Number>::build(MoosePreconditioner::_communicator));
  mdinv_r2c->init(_MDinv->m(), _MDinv->local_m(), false, PARALLEL);

  // get _r2c from the original y
  y.create_subvector(*_r2c, _gu2c);

  _MDinv->vector_mult(*mdinv_r2c, *_r2c);
  mdinv_r2c->close();

  // change values in y_copy
  std::vector<numeric_index_type> dof_indices;
  std::vector<Number> vals;
  for (auto idx = mdinv_r2c->first_local_index(); idx < mdinv_r2c->last_local_index(); ++idx)
  {
    dof_indices.push_back(_grows[idx]);
    vals.push_back(-(*mdinv_r2c)(idx)); // note the minus sign here
  }

  y_copy->add_vector(vals.data(), dof_indices);

  y_copy->create_subvector(*_y_hat, _grows);

  _y_hat->close();
  _x_hat->close();
}

void
VariableCondensationPreconditioner::computeCondensedVariables()
{
  _lambda->init(_D->m(), _D->local_m(), false, PARALLEL);

  std::unique_ptr<NumericVector<Number>> u2c_rows_x_hat(
      NumericVector<Number>::build(MoosePreconditioner::_communicator));
  u2c_rows_x_hat->init(_u2c_rows->m(), _u2c_rows->local_m(), false, PARALLEL);
  _u2c_rows->vector_mult(*u2c_rows_x_hat, *_x_hat);
  u2c_rows_x_hat->close();

  (*_r2c) -= (*u2c_rows_x_hat);
  _r2c->close();
  _D_inv->vector_mult(*_lambda, *_r2c);
  _lambda->close();
}

void
VariableCondensationPreconditioner::getFullSolution(const NumericVector<Number> & /*y*/,
                                                    NumericVector<Number> & x)
{
  std::vector<numeric_index_type> dof_indices;
  std::vector<Number> vals;

  // save values and indices from _x_hat and _lambda
  for (auto i = _x_hat->first_local_index(); i < _x_hat->last_local_index(); ++i)
  {
    dof_indices.push_back(_gcols[i]);
    vals.push_back((*_x_hat)(i));
  }

  for (auto i = _lambda->first_local_index(); i < _lambda->last_local_index(); ++i)
  {
    dof_indices.push_back(_glm[i]);
    vals.push_back((*_lambda)(i));
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
}

void
VariableCondensationPreconditioner::computeDInverse()
{
  PetscErrorCode ierr;
  Mat F, I, Dinv;
  IS perm, iperm;
  MatFactorInfo info;

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

  // Initialize Dinv
  ierr = MatCreateDense(PETSC_COMM_WORLD,
                        static_cast<PetscInt>(_D->local_n()),
                        static_cast<PetscInt>(_D->local_m()),
                        static_cast<PetscInt>(_D->n()),
                        static_cast<PetscInt>(_D->m()),
                        NULL,
                        &Dinv);
  LIBMESH_CHKERR(ierr);

  // Solve for Dinv
  ierr = MatMatSolve(F, I, Dinv);
  LIBMESH_CHKERR(ierr);

  ierr = MatAssemblyBegin(Dinv, MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);
  ierr = MatAssemblyEnd(Dinv, MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);

  // copy value to _D_inv
  ierr = MatCopy(Dinv, _D_inv->mat(), SAME_NONZERO_PATTERN);
  LIBMESH_CHKERR(ierr);

  ierr = MatDestroy(&I);
  LIBMESH_CHKERR(ierr);
  ierr = MatDestroy(&F);
  LIBMESH_CHKERR(ierr);
  ierr = MatDestroy(&Dinv);
  LIBMESH_CHKERR(ierr);
  ierr = ISDestroy(&perm);
  LIBMESH_CHKERR(ierr);
  ierr = ISDestroy(&iperm);
  LIBMESH_CHKERR(ierr);
}

void
VariableCondensationPreconditioner::computeDInverseDiag()
{
  PetscErrorCode ierr;
  auto diag_D = NumericVector<Number>::build(MoosePreconditioner::_communicator);
  // Allocate storage
  diag_D->init(_D->m(), _D->local_m(), false, PARALLEL);
  // Fill entries
  for (numeric_index_type i = _D->row_start(); i < _D->row_stop(); ++i)
    diag_D->set(_map_gu2c_to_order.at(_gu2c[i]), (*_D)(i, _map_gu2c_to_order.at(_gu2c[i])));

  for (numeric_index_type i = _D->row_start(); i < _D->row_stop(); ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual((*diag_D)(i), 0.0))
      mooseError("Trying to compute reciprocal of 0.");
    ierr = MatSetValue(_D_inv->mat(),
                       static_cast<PetscInt>(i),
                       static_cast<PetscInt>(_map_gu2c_to_order.at(_gu2c[i])),
                       static_cast<PetscScalar>(1.0 / (*diag_D)(i)),
                       INSERT_VALUES);
  }

  ierr = MatAssemblyBegin(_D_inv->mat(), MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);
  ierr = MatAssemblyEnd(_D_inv->mat(), MAT_FINAL_ASSEMBLY);
  LIBMESH_CHKERR(ierr);
}
