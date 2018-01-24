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
#include "MooseVariable.h"

// libMesh
#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"

#include <limits>

MooseVariableScalar::MooseVariableScalar(unsigned int var_num,
                                         const FEType & fe_type,
                                         SystemBase & sys,
                                         Assembly & assembly,
                                         Moose::VarKindType var_kind)
  : MooseVariableBase(var_num, fe_type, sys, assembly, var_kind)
{
}

MooseVariableScalar::~MooseVariableScalar()
{
  _u.release();
  _u_old.release();
  _u_older.release();

  _u_dot.release();
  _du_dot_du.release();
}

void
MooseVariableScalar::reinit()
{
  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();
  const Real & du_dot_du = _sys.duDotDu();

  _dof_map.SCALAR_dof_indices(_dof_indices, _var_num);

  unsigned int n = _dof_indices.size();
  _u.resize(n);
  _u_old.resize(n);
  _u_older.resize(n);
  _u_dot.resize(n);

  _du_dot_du.clear();
  _du_dot_du.resize(n, du_dot_du);

  // If we have an empty partition, or if we have a partition which
  // does not include any of the subdomains of a subdomain-restricted
  // variable, then we do not have access to that variable!  Hopefully
  // we won't need the indices we lack.
  if (_dof_map.all_semilocal_indices(_dof_indices))
  {
    current_solution.get(_dof_indices, &_u[0]);
    solution_old.get(_dof_indices, &_u_old[0]);
    solution_older.get(_dof_indices, &_u_older[0]);
    u_dot.get(_dof_indices, &_u_dot[0]);
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
        u_dot.get(one_dof_index, &_u_dot[i]);
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
        _u_dot.resize(i);
#else
        // If we can't catch errors at run-time, we can at least
        // propagate NaN values rather than invalid values, so that
        // users won't trust the result.
        _u[i] = std::numeric_limits<Real>::quiet_NaN();
        _u_old[i] = std::numeric_limits<Real>::quiet_NaN();
        _u_older[i] = std::numeric_limits<Real>::quiet_NaN();
        _u_dot[i] = std::numeric_limits<Real>::quiet_NaN();
#endif
      }
    }
  }
}

bool
MooseVariableScalar::isNodal() const
{
  // scalar variables are never nodal ;-)
  return false;
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
