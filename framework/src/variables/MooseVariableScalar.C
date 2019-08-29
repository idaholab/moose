//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "SystemBase.h"

// libMesh
#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"

#include <limits>

registerMooseObject("MooseApp", MooseVariableScalar);

template <>
InputParameters
validParams<MooseVariableScalar>()
{
  auto params = validParams<MooseVariableBase>();
  params.addClassDescription("Moose wrapper class around scalar variables");
  params.set<MooseEnum>("family") = "SCALAR";
  return params;
}

MooseVariableScalar::MooseVariableScalar(const InputParameters & parameters)
  : MooseVariableBase(parameters),
    _need_u_dot(false),
    _need_u_dotdot(false),
    _need_u_dot_old(false),
    _need_u_dotdot_old(false),
    _need_du_dot_du(false),
    _need_du_dotdot_du(false),
    _need_dual(false),
    _need_dual_u(false)
{
  auto num_vector_tags = _sys.subproblem().numVectorTags();

  _vector_tag_u.resize(num_vector_tags);
  _need_vector_tag_u.resize(num_vector_tags);

  auto num_matrix_tags = _sys.subproblem().numMatrixTags();

  _matrix_tag_u.resize(num_matrix_tags);
  _need_matrix_tag_u.resize(num_matrix_tags);
}

MooseVariableScalar::~MooseVariableScalar()
{
  _u.release();
  _u_old.release();
  _u_older.release();

  _u_dot.release();
  _u_dotdot.release();
  _u_dot_old.release();
  _u_dotdot_old.release();
  _du_dot_du.release();
  _du_dotdot_du.release();

  for (auto & _tag_u : _vector_tag_u)
    _tag_u.release();

  _vector_tag_u.clear();

  for (auto & _tag_u : _matrix_tag_u)
    _tag_u.release();

  _matrix_tag_u.clear();
}

void
MooseVariableScalar::reinit(bool reinit_for_derivative_reordering /* = false*/)
{
  if (reinit_for_derivative_reordering)
  {
    // We've already calculated everything in an earlier reinit. All we have to do is re-sort the
    // derivative vector for AD calculations
    if (_need_dual && _subproblem.currentlyComputingJacobian())
      computeAD(/*nodal_ordering=*/true);
    return;
  }

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> * u_dot = _sys.solutionUDot();
  const NumericVector<Real> * u_dotdot = _sys.solutionUDotDot();
  const NumericVector<Real> * u_dot_old = _sys.solutionUDotOld();
  const NumericVector<Real> * u_dotdot_old = _sys.solutionUDotDotOld();
  const Real & du_dot_du = _sys.duDotDu();
  const Real & du_dotdot_du = _sys.duDotDotDu();
  auto safe_access_tagged_vectors = _sys.subproblem().safeAccessTaggedVectors();
  auto safe_access_tagged_matrices = _sys.subproblem().safeAccessTaggedMatrices();
  auto & active_coupleable_matrix_tags =
      _sys.subproblem().getActiveScalarVariableCoupleableMatrixTags(_tid);
  auto & active_coupleable_vector_tags =
      _sys.subproblem().getActiveScalarVariableCoupleableVectorTags(_tid);

  _dof_map.SCALAR_dof_indices(_dof_indices, _var_num);

  auto n = _dof_indices.size();
  _u.resize(n);
  _u_old.resize(n);
  _u_older.resize(n);

  for (auto & _tag_u : _vector_tag_u)
    _tag_u.resize(n);

  for (auto & _tag_u : _matrix_tag_u)
    _tag_u.resize(n);

  _du_dot_du.clear();
  _du_dot_du.resize(n, du_dot_du);

  if (_need_u_dot)
    _u_dot.resize(n);

  if (_need_u_dotdot)
    _u_dotdot.resize(n);

  if (_need_u_dot_old)
    _u_dot_old.resize(n);

  if (_need_u_dotdot_old)
    _u_dotdot_old.resize(n);

  if (_need_du_dot_du)
  {
    _du_dot_du.clear();
    _du_dot_du.resize(n, du_dot_du);
  }

  if (_need_du_dotdot_du)
  {
    _du_dotdot_du.clear();
    _du_dotdot_du.resize(n, du_dotdot_du);
  }

  // If we have an empty partition, or if we have a partition which
  // does not include any of the subdomains of a subdomain-restricted
  // variable, then we do not have access to that variable!  Hopefully
  // we won't need the indices we lack.
  if (_dof_map.all_semilocal_indices(_dof_indices))
  {
    current_solution.get(_dof_indices, &_u[0]);
    solution_old.get(_dof_indices, &_u_old[0]);
    solution_older.get(_dof_indices, &_u_older[0]);

    if (safe_access_tagged_vectors)
    {
      for (auto tag : active_coupleable_vector_tags)
        if (_sys.hasVector(tag) && _need_vector_tag_u[tag])
          _sys.getVector(tag).get(_dof_indices, &_vector_tag_u[tag][0]);
    }

    if (safe_access_tagged_matrices)
    {
      for (auto tag : active_coupleable_matrix_tags)
        if (_sys.hasMatrix(tag) && _sys.getMatrix(tag).closed() && _need_matrix_tag_u[tag])
          for (std::size_t i = 0; i != n; ++i)
          {
            Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
            _matrix_tag_u[tag][i] = _sys.getMatrix(tag)(_dof_indices[i], _dof_indices[i]);
          }
    }

    if (_need_u_dot)
      (*u_dot).get(_dof_indices, &_u_dot[0]);

    if (_need_u_dotdot)
      (*u_dotdot).get(_dof_indices, &_u_dotdot[0]);

    if (_need_u_dot_old)
      (*u_dot_old).get(_dof_indices, &_u_dot_old[0]);

    if (_need_u_dotdot_old)
      (*u_dotdot_old).get(_dof_indices, &_u_dotdot_old[0]);
  }
  else
  {
    for (std::size_t i = 0; i != n; ++i)
    {
      const dof_id_type dof_index = _dof_indices[i];
      std::vector<dof_id_type> one_dof_index(1, dof_index);
      if (_dof_map.all_semilocal_indices(one_dof_index))
      {
        libmesh_assert_less(i, _u.size());

        current_solution.get(one_dof_index, &_u[i]);
        solution_old.get(one_dof_index, &_u_old[i]);
        solution_older.get(one_dof_index, &_u_older[i]);

        if (safe_access_tagged_vectors)
        {
          for (auto tag : active_coupleable_vector_tags)
            if (_sys.hasVector(tag) && _need_vector_tag_u[tag])
              _sys.getVector(tag).get(one_dof_index, &_vector_tag_u[tag][i]);
        }

        if (safe_access_tagged_matrices)
        {
          for (auto tag : active_coupleable_matrix_tags)
            if (_sys.hasMatrix(tag) && _sys.getMatrix(tag).closed() && _need_matrix_tag_u[tag])
            {
              Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
              _matrix_tag_u[tag][i] = _sys.getMatrix(tag)(dof_index, dof_index);
            }
        }

        if (_need_u_dot)
          (*u_dot).get(one_dof_index, &_u_dot[i]);

        if (_need_u_dotdot)
          (*u_dotdot).get(one_dof_index, &_u_dotdot[i]);

        if (_need_u_dot_old)
          (*u_dot_old).get(one_dof_index, &_u_dot_old[i]);

        if (_need_u_dotdot_old)
          (*u_dotdot_old).get(one_dof_index, &_u_dotdot_old[i]);
      }
      else
      {
#ifdef _GLIBCXX_DEBUG
        // Let's make it possible to catch invalid accesses to these
        // variables immediately via a thrown exception, if our
        // libstdc++ compiler flags allow for that.
        _u.resize(i);
        _u_old.resize(i);
        _u_older.resize(i);

        for (auto tag : active_coupleable_vector_tags)
          if (_sys.hasVector(tag) && _need_vector_tag_u[tag])
            _vector_tag_u[tag].resize(i);

        for (auto tag : active_coupleable_matrix_tags)
          if (_sys.hasMatrix(tag) && _sys.getMatrix(tag).closed() && _need_matrix_tag_u[tag])
            _matrix_tag_u[tag].resize(i);

        if (_need_u_dot)
          _u_dot.resize(i);

        if (_need_u_dotdot)
          _u_dotdot.resize(i);

        if (_need_u_dot_old)
          _u_dot_old.resize(i);

        if (_need_u_dotdot_old)
          _u_dotdot_old.resize(i);
#else
        // If we can't catch errors at run-time, we can at least
        // propagate NaN values rather than invalid values, so that
        // users won't trust the result.
        _u[i] = std::numeric_limits<Real>::quiet_NaN();
        _u_old[i] = std::numeric_limits<Real>::quiet_NaN();
        _u_older[i] = std::numeric_limits<Real>::quiet_NaN();

        for (auto tag : active_coupleable_vector_tags)
          if (_sys.hasVector(tag) && _need_vector_tag_u[tag])
            _vector_tag_u[tag][i] = std::numeric_limits<Real>::quiet_NaN();

        for (auto tag : active_coupleable_matrix_tags)
          if (_sys.hasMatrix(tag) && _sys.getMatrix(tag).closed() && _need_matrix_tag_u[tag])
            _matrix_tag_u[tag][i] = std::numeric_limits<Real>::quiet_NaN();

        if (_need_u_dot)
          _u_dot[i] = std::numeric_limits<Real>::quiet_NaN();

        if (_need_u_dotdot)
          _u_dotdot[i] = std::numeric_limits<Real>::quiet_NaN();

        if (_need_u_dot_old)
          _u_dot_old[i] = std::numeric_limits<Real>::quiet_NaN();

        if (_need_u_dotdot_old)
          _u_dotdot_old[i] = std::numeric_limits<Real>::quiet_NaN();
#endif
      }
    }
  }

  // Automatic differentiation
  if (_need_dual && _subproblem.currentlyComputingJacobian())
    computeAD(/*nodal_ordering=*/false);
}

void
MooseVariableScalar::computeAD(bool nodal_ordering)
{
  auto ad_offset =
      _var_num * (nodal_ordering ? _sys.getMaxVarNDofsPerNode() : _sys.getMaxVarNDofsPerElem());
  auto n_dofs = _dof_indices.size();

  if (_need_dual_u)
  {
    _dual_u.resize(n_dofs);
    for (MooseIndex(n_dofs) i = 0; i < n_dofs; ++i)
    {
      _dual_u[i] = _u[i];
      Moose::derivInsert(_dual_u[i].derivatives(), ad_offset + i, 1.);
    }
  }
}

void
MooseVariableScalar::setValue(unsigned int i, Number value)
{
// In debug modes, we might have set a "trap" to catch reads of
// uninitialized values, but this trap shouldn't prevent setting
// values.
#ifdef DEBUG
  if (i >= _u.size())
  {
    libmesh_assert_less(i, _dof_indices.size());
    _u.resize(i + 1);
  }
#endif
  _u[i] = value; // update variable value
}

void
MooseVariableScalar::setValues(Number value)
{
  unsigned int n = _dof_indices.size();
// In debug modes, we might have set a "trap" to catch reads of
// uninitialized values, but this trap shouldn't prevent setting
// values.
#ifdef DEBUG
  _u.resize(n);
#endif
  for (unsigned int i = 0; i < n; i++)
    _u[i] = value;
}

void
MooseVariableScalar::insert(NumericVector<Number> & soln)
{
  // We may have redundantly computed this value on many different
  // processors, but only the processor which actually owns it should
  // be saving it to the solution vector, to avoid O(N_scalar_vars)
  // unnecessary communication.

  const dof_id_type first_dof = _dof_map.first_dof();
  const dof_id_type end_dof = _dof_map.end_dof();
  if (_dof_indices.size() > 0 && first_dof <= _dof_indices[0] && _dof_indices[0] < end_dof)
    soln.insert(&_u[0], _dof_indices);
}

template <>
const VariableValue &
MooseVariableScalar::adSln<ComputeStage::RESIDUAL>() const
{
  return _u;
}

template <>
const DualVariableValue &
MooseVariableScalar::adSln<ComputeStage::JACOBIAN>() const
{
  _need_dual = _need_dual_u = true;
  return _dual_u;
}
