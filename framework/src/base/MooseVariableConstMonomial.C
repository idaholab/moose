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
                                                       Moose::VarKindType var_kind)
  : MooseVariable(var_num, fe_type, sys, assembly, var_kind)
{
}

void
MooseVariableConstMonomial::computeElemValuesHelper(const unsigned & nqp, const Real & phi)
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

  const dof_id_type & idx = _dof_indices[0];
  const Real & soln = (*_sys.currentSolution())(idx);
  Real soln_old;
  Real soln_older;
  Real soln_previous_nl;
  Real u_dot;
  const Real & du_dot_du = _sys.duDotDu();

  if (_need_nodal_u)
    _nodal_u[0] = soln;

  if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
      _need_nodal_u_previous_nl)
    soln_previous_nl = (*_sys.solutionPreviousNewton())(idx);

  if (_need_nodal_u_previous_nl)
    _nodal_u_previous_nl[0] = soln_previous_nl;

  if (_need_solution_dofs)
    _solution_dofs(0) = soln;

  if (is_transient)
  {
    if (_need_u_old || _need_grad_old || _need_second_old || _need_nodal_u_old)
      soln_old = _sys.solutionOld()(idx);

    if (_need_u_older || _need_grad_older || _need_second_older || _need_nodal_u_older)
      soln_older = _sys.solutionOlder()(idx);

    if (_need_nodal_u_old)
      _nodal_u_old[0] = soln_old;

    if (_need_nodal_u_older)
      _nodal_u_older[0] = soln_older;

    u_dot = _sys.solutionUDot()(idx);

    if (_need_nodal_u_dot)
      _nodal_u_dot[0] = u_dot;

    if (_need_solution_dofs_old)
      _solution_dofs_old(0) = soln_old;

    if (_need_solution_dofs_older)
      _solution_dofs_older(0) = soln_older;
  }

  _u[0] = phi * soln;

  if (_need_u_previous_nl)
    _u_previous_nl[0] = phi * soln_previous_nl;

  if (is_transient)
  {
    _u_dot[0] = phi * u_dot;
    _du_dot_du[0] = du_dot_du;

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
      _u_dot[qp] = _u_dot[0];
      _du_dot_du[qp] = _du_dot_du[0];

      if (_need_u_old)
        _u_old[qp] = _u_old[0];

      if (_need_u_older)
        _u_older[qp] = _u_older[qp];
    }
  }
}

void
MooseVariableConstMonomial::computeNeighborValuesHelper(const unsigned & nqp, const Real & phi)
{
  bool is_transient = _subproblem.isTransient();

  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);

  if (_need_second_neighbor)
    _second_u_neighbor.resize(nqp);

  if (is_transient)
  {
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

  if (_need_nodal_u_neighbor)
    _nodal_u_neighbor.resize(1);
  if (is_transient)
  {
    if (_need_nodal_u_old_neighbor)
      _nodal_u_old_neighbor.resize(1);
    if (_need_nodal_u_older_neighbor)
      _nodal_u_older_neighbor.resize(1);
    if (_need_nodal_u_dot_neighbor)
      _nodal_u_dot_neighbor.resize(1);
  }

  if (_need_solution_dofs_neighbor)
    _solution_dofs_neighbor.resize(1);

  if (_need_solution_dofs_old_neighbor)
    _solution_dofs_old_neighbor.resize(1);

  if (_need_solution_dofs_older_neighbor)
    _solution_dofs_older_neighbor.resize(1);

  const dof_id_type & idx = _dof_indices_neighbor[0];
  const Real & soln = (*_sys.currentSolution())(idx);
  Real soln_old;
  Real soln_older;
  Real u_dot;

  if (_need_nodal_u_neighbor)
    _nodal_u_neighbor[0] = soln;

  if (_need_solution_dofs_neighbor)
    _solution_dofs_neighbor(0) = soln;

  if (is_transient)
  {
    if (_need_u_old_neighbor)
      soln_old = _sys.solutionOld()(idx);

    if (_need_u_older_neighbor)
      soln_older = _sys.solutionOlder()(idx);

    if (_need_nodal_u_old_neighbor)
      _nodal_u_old_neighbor[0] = soln_old;
    if (_need_nodal_u_older_neighbor)
      _nodal_u_older_neighbor[0] = soln_older;

    u_dot = _sys.solutionUDot()(idx);

    if (_need_nodal_u_dot_neighbor)
      _nodal_u_dot_neighbor[0] = u_dot;

    if (_need_solution_dofs_old_neighbor)
      _solution_dofs_old_neighbor(0) = soln_old;

    if (_need_solution_dofs_older_neighbor)
      _solution_dofs_older_neighbor(0) = soln_older;
  }

  _u_neighbor[0] = phi * soln;

  if (is_transient)
  {
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

  computeElemValuesHelper(_qrule->n_points(), _phi[0][0]);
}

void
MooseVariableConstMonomial::computeElemValuesFace()
{
  if (_dof_indices.size() == 0)
    return;

  computeElemValuesHelper(_qrule_face->n_points(), _phi_face[0][0]);
}

void
MooseVariableConstMonomial::computeNeighborValues()
{
  if (_dof_indices_neighbor.size() == 0)
    return;

  computeNeighborValuesHelper(_qrule_neighbor->n_points(), _phi_neighbor[0][0]);
}

void
MooseVariableConstMonomial::computeNeighborValuesFace()
{
  if (_dof_indices_neighbor.size() == 0)
    return;

  computeNeighborValuesHelper(_qrule_neighbor->n_points(), _phi_face_neighbor[0][0]);
}
