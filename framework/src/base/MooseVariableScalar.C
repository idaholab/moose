/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"

// libMesh
#include "libmesh/numeric_vector.h"

MooseVariableScalar::MooseVariableScalar(unsigned int var_num, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind) :
    MooseVariableBase(var_num, sys, assembly, var_kind)
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
  const NumericVector<Real> & solution_old     = _sys.solutionOld();
  const NumericVector<Real> & solution_older   = _sys.solutionOlder();
  const NumericVector<Real> & u_dot            = _sys.solutionUDot();
  const NumericVector<Real> & du_dot_du        = _sys.solutionDuDotDu();

  _dof_map.SCALAR_dof_indices(_dof_indices, _var_num);

  dof_id_type n = _dof_indices.size();
  _u.resize(n);
  _u_old.resize(n);
  _u_older.resize(n);
  if (_is_nl)
  {
    _u_dot.resize(n);
    _du_dot_du.resize(n);
  }

  for (dof_id_type i = 0; i < n; i++)
  {
    dof_id_type idx = _dof_indices[i];
    _u[i] = current_solution(idx);
    _u_old[i] = solution_old(idx);
    _u_older[i] = solution_older(idx);

    if (_is_nl)
    {
      _u_dot[i]        = u_dot(idx);
      _du_dot_du[i]    = du_dot_du(idx);
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
MooseVariableScalar::setValue(dof_id_type i, Number value)
{
  _u[i] = value;                  // update variable value
}

void
MooseVariableScalar::insert(NumericVector<Number> & soln)
{
  for (dof_id_type i = 0; i < _dof_indices.size(); i++)
    soln.set(_dof_indices[i], _u[i]);
}
