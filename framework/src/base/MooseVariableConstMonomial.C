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

#include "MooseVariableConstMonomial.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

MooseVariableConstMonomial::MooseVariableConstMonomial(unsigned int var_num,
                                                       const FEType & fe_type,
                                                       SystemBase & sys,
                                                       Assembly & assembly,
                                                       Moose::VarKindType var_kind)
  : MooseVariable(var_num, fe_type, sys, assembly, var_kind)
{
}

void
MooseVariableConstMonomial::computeElemValues()
{

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _qrule->n_points();

  _u.resize(nqp);
  _grad_u.resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (_need_u_previous_nl)
    _u_previous_nl.resize(nqp);

  if (_need_grad_previous_nl)
    _grad_u_previous_nl.resize(nqp);

  if (_need_second_previous_nl)
    _second_u_previous_nl.resize(nqp);

  if (is_transient)
  {
    _u_dot.resize(nqp);
    _du_dot_du.resize(nqp);

    if (_need_u_old)
      _u_old.resize(nqp);

    if (_need_u_older)
      _u_older.resize(nqp);

    if (_need_grad_old)
      _grad_u_old.resize(nqp);

    if (_need_grad_older)
      _grad_u_older.resize(nqp);

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  if (_need_nodal_u)
    _nodal_u.resize(1);

  if (_need_nodal_u_previous_nl)
    _nodal_u_previous_nl.resize(1);

  if (is_transient)
  {
    if (_need_nodal_u_old)
      _nodal_u_old.resize(1);
    if (_need_nodal_u_older)
      _nodal_u_older.resize(1);
    if (_need_nodal_u_dot)
      _nodal_u_dot.resize(1);
  }

  if (_need_solution_dofs)
    _solution_dofs.resize(1);

  if (_need_solution_dofs_old)
    _solution_dofs_old.resize(1);

  if (_need_solution_dofs_older)
    _solution_dofs_older.resize(1);

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> * solution_prev_nl = _sys.solutionPreviousNewton();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();
  const Real & du_dot_du = _sys.duDotDu();

  dof_id_type idx = 0;
  Real soln_local = 0;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real soln_previous_nl_local = 0;
  Real u_dot_local = 0;

  Real phi_local = 0;

  idx = _dof_indices[0];
  soln_local = current_solution(idx);

  if (_need_nodal_u)
    _nodal_u[0] = soln_local;

  if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
      _need_nodal_u_previous_nl)
    soln_previous_nl_local = (*solution_prev_nl)(idx);

  if (_need_nodal_u_previous_nl)
    _nodal_u_previous_nl[0] = soln_previous_nl_local;

  if (_need_solution_dofs)
    _solution_dofs(0) = soln_local;

  if (is_transient)
  {
    if (_need_u_old || _need_grad_old || _need_second_old || _need_nodal_u_old)
      soln_old_local = solution_old(idx);

    if (_need_u_older || _need_grad_older || _need_second_older || _need_nodal_u_older)
      soln_older_local = solution_older(idx);

    if (_need_nodal_u_old)
      _nodal_u_old[0] = soln_old_local;
    if (_need_nodal_u_older)
      _nodal_u_older[0] = soln_older_local;

    u_dot_local = u_dot(idx);
    if (_need_nodal_u_dot)
      _nodal_u_dot[0] = u_dot_local;

    if (_need_solution_dofs_old)
      _solution_dofs_old(0) = solution_old(idx);

    if (_need_solution_dofs_older)
      _solution_dofs_older(0) = solution_older(idx);
  }

  phi_local = _phi[0][0];

  _u[0] = phi_local * soln_local;

  if (_need_u_previous_nl)
    _u_previous_nl[0] = phi_local * soln_previous_nl_local;

  if (is_transient)
  {
    _u_dot[0] = phi_local * u_dot_local;
    _du_dot_du[0] = du_dot_du;

    if (_need_u_old)
      _u_old[0] = phi_local * soln_old_local;

    if (_need_u_older)
      _u_older[0] = phi_local * soln_older_local;
  }

  for (unsigned qp = 1; qp < nqp; ++qp)
  {
    _u[qp] = _u[0];

    if (_need_u_previous_nl)
      _u_previous_nl[qp] = _u_previous_nl[0];

    if (is_transient)
    {
      _u_dot[qp] = _u_dot[0];
      _du_dot_du[qp] = _du_dot_du[0];

      if (_need_u_old)
        _u_old[qp] = _u_old[0];

      if (_need_u_older)
        _u_older[qp] = _u_older[qp];
    }
  }
}
