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
#include "MooseVariable.h"

// libMesh
#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"

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
  // we won't need it.
  if (_dof_map.all_semilocal_indices(_dof_indices))
  {
    current_solution.get(_dof_indices, &_u[0]);
    solution_old.get(_dof_indices, &_u_old[0]);
    solution_older.get(_dof_indices, &_u_older[0]);
    u_dot.get(_dof_indices, &_u_dot[0]);
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
  _u[i] = value; // update variable value
}

void
MooseVariableScalar::setValues(Number value)
{
  unsigned int n = _dof_indices.size();
  for (unsigned int i = 0; i < n; i++)
    _u[i] = value;
}

void
MooseVariableScalar::insert(NumericVector<Number> & soln)
{
  if (_dof_indices.size() > 0)
    soln.insert(&_u[0], _dof_indices);
}
