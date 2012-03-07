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
#include "numeric_vector.h"
#include "dof_map.h"

MooseVariableScalar::MooseVariableScalar(unsigned int var_num, unsigned int mvn, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind) :
    _var_num(var_num),
    _moose_var_num(mvn),
    _var_kind(var_kind),
    _subproblem(sys.subproblem()),
    _sys(sys),
    _assembly(assembly),
    _dof_map(sys.dofMap()),
    _scaling_factor(1.0),
    _is_nl(dynamic_cast<NonlinearSystem *>(&sys) != NULL)
{
}

MooseVariableScalar::~MooseVariableScalar()
{
  _u.release();
  _u_old.release();

  _u_dot.release();
  _du_dot_du.release();
}

const std::string &
MooseVariableScalar::name()
{
  return _sys.system().variable(_var_num).name();
}

unsigned int
MooseVariableScalar::order() const
{
  return static_cast<unsigned int>(_sys.system().variable_type(_var_num).order);
}

void
MooseVariableScalar::reinit()
{
  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old     = _sys.solutionOld();
  const NumericVector<Real> & u_dot            = _sys.solutionUDot();
  const NumericVector<Real> & du_dot_du        = _sys.solutionDuDotDu();

  _dof_map.SCALAR_dof_indices(_dof_indices, _var_num);

  unsigned int n = _dof_indices.size();
  _u.resize(n);
  _u_old.resize(n);
  if (_is_nl)
  {
    _u_dot.resize(n);
    _du_dot_du.resize(n);
  }

  for (unsigned int i = 0; i < n; i++)
  {
    unsigned int idx = _dof_indices[i];
    _u[i] = current_solution(idx);
    _u_old[i] = solution_old(idx);

    if (_is_nl)
    {
      _u_dot[i]        = u_dot(idx);
      _du_dot_du[i]    = du_dot_du(idx);
    }
  }
}

void
MooseVariableScalar::setValue(unsigned int i, Number value)
{
  _u[i] = value;                  // update variable value
}

void
MooseVariableScalar::insert(NumericVector<Number> & soln)
{
  for (unsigned int i = 0; i < _dof_indices.size(); i++)
    soln.set(_dof_indices[i], _u[i]);
}
