//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableConstMonomial.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

MooseVariableConstMonomial::MooseVariableConstMonomial(unsigned int var_num,
                                                       const FEType & fe_type,
                                                       SystemBase & sys,
                                                       Assembly & assembly,
                                                       Moose::VarKindType var_kind,
                                                       THREAD_ID tid)
  : MooseVariable(var_num, fe_type, sys, assembly, var_kind, tid)
{
}

void
MooseVariableConstMonomial::computeMonomialValuesHelper(const unsigned & nqp, const Real & phi)
{
  bool is_transient = _subproblem.isTransient();

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
    if (_need_u_dot)
      _u_dot.resize(nqp);

    if (_need_u_dotdot)
      _u_dotdot.resize(nqp);

    if (_need_u_dot_old)
      _u_dot_old.resize(nqp);

    if (_need_u_dotdot_old)
      _u_dotdot_old.resize(nqp);

    if (_need_du_dot_du)
      _du_dot_du.resize(nqp);

    if (_need_du_dotdot_du)
      _du_dotdot_du.resize(nqp);

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

  if (_need_dof_values)
    _dof_values.resize(1);

  if (_need_dof_values_previous_nl)
    _dof_values_previous_nl.resize(1);

  if (is_transient)
  {
    if (_need_dof_values_old)
      _dof_values_old.resize(1);
    if (_need_dof_values_older)
      _dof_values_older.resize(1);
    if (_need_dof_values_dot)
      _dof_values_dot.resize(1);
    if (_need_dof_values_dotdot)
      _dof_values_dotdot.resize(1);
    if (_need_dof_values_dot_old)
      _dof_values_dot_old.resize(1);
    if (_need_dof_values_dotdot_old)
      _dof_values_dotdot_old.resize(1);
  }

  const dof_id_type & idx = _dof_indices[0];
  const Real & soln = (*_sys.currentSolution())(idx);
  Real soln_old = 0;
  Real soln_older = 0;
  Real soln_previous_nl = 0;
  Real u_dot = 0;
  Real u_dotdot = 0;
  Real u_dot_old = 0;
  Real u_dotdot_old = 0;
  const Real & du_dot_du = _sys.duDotDu();
  const Real & du_dotdot_du = _sys.duDotDotDu();

  if (_need_dof_values)
    _dof_values[0] = soln;

  if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
      _need_dof_values_previous_nl)
    soln_previous_nl = (*_sys.solutionPreviousNewton())(idx);

  if (_need_dof_values_previous_nl)
    _dof_values_previous_nl[0] = soln_previous_nl;

  if (is_transient)
  {
    if (_need_u_old || _need_grad_old || _need_second_old || _need_dof_values_old)
      soln_old = _sys.solutionOld()(idx);

    if (_need_u_older || _need_grad_older || _need_second_older || _need_dof_values_older)
      soln_older = _sys.solutionOlder()(idx);

    if (_need_dof_values_old)
      _dof_values_old[0] = soln_old;

    if (_need_dof_values_older)
      _dof_values_older[0] = soln_older;

    if (_sys.solutionUDot())
      u_dot = (*_sys.solutionUDot())(idx);
    if (_sys.solutionUDotDot())
      u_dotdot = (*_sys.solutionUDotDot())(idx);
    if (_sys.solutionUDotOld())
      u_dot_old = (*_sys.solutionUDotOld())(idx);
    if (_sys.solutionUDotDotOld())
      u_dotdot_old = (*_sys.solutionUDotDotOld())(idx);

    if (_need_dof_values_dot)
      _dof_values_dot[0] = u_dot;

    if (_need_dof_values_dotdot)
      _dof_values_dotdot[0] = u_dotdot;
  }

  _u[0] = phi * soln;

  if (_need_u_previous_nl)
    _u_previous_nl[0] = phi * soln_previous_nl;

  if (is_transient)
  {
    if (_need_u_dot)
      _u_dot[0] = phi * u_dot;

    if (_need_u_dotdot)
      _u_dotdot[0] = phi * u_dotdot;

    if (_need_u_dot_old)
      _u_dot_old[0] = phi * u_dot_old;

    if (_need_u_dotdot_old)
      _u_dotdot_old[0] = phi * u_dotdot_old;

    if (_need_du_dot_du)
      _du_dot_du[0] = du_dot_du;

    if (_need_du_dotdot_du)
      _du_dotdot_du[0] = du_dotdot_du;

    if (_need_u_old)
      _u_old[0] = phi * soln_old;

    if (_need_u_older)
      _u_older[0] = phi * soln_older;
  }

  for (unsigned qp = 1; qp < nqp; ++qp)
  {
    _u[qp] = _u[0];

    if (_need_u_previous_nl)
      _u_previous_nl[qp] = _u_previous_nl[0];

    if (is_transient)
    {
      if (_need_u_dot)
        _u_dot[qp] = _u_dot[0];

      if (_need_u_dotdot)
        _u_dotdot[qp] = _u_dotdot[0];

      if (_need_u_dot_old)
        _u_dot_old[qp] = _u_dot_old[0];

      if (_need_u_dotdot_old)
        _u_dotdot_old[qp] = _u_dotdot_old[0];

      if (_need_du_dot_du)
        _du_dot_du[qp] = _du_dot_du[0];

      if (_need_du_dotdot_du)
        _du_dotdot_du[qp] = _du_dotdot_du[0];

      if (_need_u_old)
        _u_old[qp] = _u_old[0];

      if (_need_u_older)
        _u_older[qp] = _u_older[qp];
    }
  }
}

void
MooseVariableConstMonomial::computeMonomialNeighborValuesHelper(const unsigned & nqp,
                                                                const Real & phi)
{
  bool is_transient = _subproblem.isTransient();

  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);

  if (_need_second_neighbor)
    _second_u_neighbor.resize(nqp);

  if (is_transient)
  {
    if (_need_u_dot_neighbor)
      _u_dot_neighbor.resize(nqp);

    if (_need_u_dotdot_neighbor)
      _u_dotdot_neighbor.resize(nqp);

    if (_need_u_dot_old_neighbor)
      _u_dot_old_neighbor.resize(nqp);

    if (_need_u_dotdot_old_neighbor)
      _u_dotdot_old_neighbor.resize(nqp);

    if (_need_du_dot_du_neighbor)
      _du_dot_du_neighbor.resize(nqp);

    if (_need_du_dotdot_du_neighbor)
      _du_dotdot_du_neighbor.resize(nqp);

    if (_need_u_old_neighbor)
      _u_old_neighbor.resize(nqp);

    if (_need_u_older_neighbor)
      _u_older_neighbor.resize(nqp);

    if (_need_grad_old_neighbor)
      _grad_u_old_neighbor.resize(nqp);

    if (_need_grad_older_neighbor)
      _grad_u_older_neighbor.resize(nqp);

    if (_need_second_old_neighbor)
      _second_u_old_neighbor.resize(nqp);

    if (_need_second_older_neighbor)
      _second_u_older_neighbor.resize(nqp);
  }

  if (_need_dof_values_neighbor)
    _dof_values_neighbor.resize(1);
  if (is_transient)
  {
    if (_need_dof_values_old_neighbor)
      _dof_values_old_neighbor.resize(1);
    if (_need_dof_values_older_neighbor)
      _dof_values_older_neighbor.resize(1);
    if (_need_dof_values_dot_neighbor)
      _dof_values_dot_neighbor.resize(1);
    if (_need_dof_values_dotdot_neighbor)
      _dof_values_dotdot_neighbor.resize(1);
    if (_need_dof_values_dot_old_neighbor)
      _dof_values_dot_old_neighbor.resize(1);
    if (_need_dof_values_dotdot_old_neighbor)
      _dof_values_dotdot_old_neighbor.resize(1);
  }

  const dof_id_type & idx = _dof_indices_neighbor[0];
  const Real & soln = (*_sys.currentSolution())(idx);
  Real soln_old = 0;
  Real soln_older = 0;
  Real u_dot = 0;
  Real u_dotdot = 0;
  Real u_dot_old = 0;
  Real u_dotdot_old = 0;
  const Real & du_dot_du = _sys.duDotDu();
  const Real & du_dotdot_du = _sys.duDotDotDu();

  if (_need_dof_values_neighbor)
    _dof_values_neighbor[0] = soln;

  if (is_transient)
  {
    if (_need_u_old_neighbor || _need_grad_old_neighbor || _need_second_old_neighbor ||
        _need_dof_values_old_neighbor)
      soln_old = _sys.solutionOld()(idx);

    if (_need_u_older_neighbor || _need_grad_older_neighbor || _need_second_older_neighbor ||
        _need_dof_values_older_neighbor)
      soln_older = _sys.solutionOlder()(idx);

    if (_need_dof_values_old_neighbor)
      _dof_values_old_neighbor[0] = soln_old;
    if (_need_dof_values_older_neighbor)
      _dof_values_older_neighbor[0] = soln_older;

    if (_sys.solutionUDot())
      u_dot = (*_sys.solutionUDot())(idx);
    if (_sys.solutionUDotDot())
      u_dotdot = (*_sys.solutionUDotDot())(idx);
    if (_sys.solutionUDotOld())
      u_dot_old = (*_sys.solutionUDotOld())(idx);
    if (_sys.solutionUDotDotOld())
      u_dotdot_old = (*_sys.solutionUDotDotOld())(idx);

    if (_need_dof_values_dot_neighbor)
      _dof_values_dot_neighbor[0] = u_dot;

    if (_need_dof_values_dotdot_neighbor)
      _dof_values_dotdot_neighbor[0] = u_dotdot;

    if (_need_dof_values_dot_old_neighbor)
      _dof_values_dot_old_neighbor[0] = u_dot_old;

    if (_need_dof_values_dotdot_old_neighbor)
      _dof_values_dotdot_old_neighbor[0] = u_dotdot_old;
  }

  _u_neighbor[0] = phi * soln;

  if (is_transient)
  {
    if (_need_u_dot_neighbor)
      _u_dot_neighbor[0] = phi * u_dot;

    if (_need_u_dotdot_neighbor)
      _u_dotdot_neighbor[0] = phi * u_dotdot;

    if (_need_u_dot_old_neighbor)
      _u_dot_old_neighbor[0] = phi * u_dot_old;

    if (_need_u_dotdot_old_neighbor)
      _u_dotdot_old_neighbor[0] = phi * u_dotdot_old;

    if (_need_du_dot_du_neighbor)
      _du_dot_du_neighbor[0] = du_dot_du;

    if (_need_du_dotdot_du_neighbor)
      _du_dotdot_du_neighbor[0] = du_dotdot_du;

    if (_need_u_old_neighbor)
      _u_old_neighbor[0] = phi * soln_old;

    if (_need_u_older_neighbor)
      _u_older_neighbor[0] = phi * soln_older;
  }

  for (unsigned qp = 1; qp < nqp; ++qp)
  {
    _u_neighbor[qp] = _u_neighbor[0];

    if (is_transient)
    {
      if (_need_u_dot_neighbor)
        _u_dot_neighbor[qp] = _u_dot_neighbor[0];

      if (_need_u_dotdot_neighbor)
        _u_dotdot_neighbor[qp] = _u_dotdot_neighbor[0];

      if (_need_u_dot_old_neighbor)
        _u_dot_old_neighbor[qp] = _u_dot_old_neighbor[0];

      if (_need_u_dotdot_old_neighbor)
        _u_dotdot_old_neighbor[qp] = _u_dotdot_old_neighbor[0];

      if (_need_du_dot_du_neighbor)
        _du_dot_du_neighbor[qp] = _du_dot_du_neighbor[0];

      if (_need_du_dotdot_du_neighbor)
        _du_dotdot_du_neighbor[qp] = _du_dotdot_du_neighbor[0];

      if (_need_u_old_neighbor)
        _u_old_neighbor[qp] = _u_old_neighbor[0];

      if (_need_u_older_neighbor)
        _u_older_neighbor[qp] = _u_older_neighbor[0];
    }
  }
}

void
MooseVariableConstMonomial::computeElemValues()
{
  if (_dof_indices.size() == 0)
    return;

  computeMonomialValuesHelper(_qrule->n_points(), _phi[0][0]);
}

void
MooseVariableConstMonomial::computeElemValuesFace()
{
  if (_dof_indices.size() == 0)
    return;

  computeMonomialValuesHelper(_qrule_face->n_points(), _phi_face[0][0]);
}

void
MooseVariableConstMonomial::computeNeighborValues()
{
  if (_dof_indices_neighbor.size() == 0)
    return;

  computeMonomialNeighborValuesHelper(_qrule_neighbor->n_points(), _phi_neighbor[0][0]);
}

void
MooseVariableConstMonomial::computeNeighborValuesFace()
{
  if (_dof_indices_neighbor.size() == 0)
    return;

  computeMonomialNeighborValuesHelper(_qrule_neighbor->n_points(), _phi_face_neighbor[0][0]);
}
