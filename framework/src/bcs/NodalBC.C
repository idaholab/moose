//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalBC.h"

#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "NonlinearSystemBase.h"

InputParameters
NodalBC::validParams()
{
  InputParameters params = NodalBCBase::validParams();

  return params;
}

NodalBC::NodalBC(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<Real>(this,
                                 true,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(*mooseVariable()),
    _current_node(_var.node()),
    _u(_var.dofValues())
{
  addMooseVariableDependency(mooseVariable());

  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i = 0; i < _diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _diag_save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "diag_save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

void
NodalBC::computeResidual()
{
  if (_var.isNodalDefined())
  {
    const Real res = computeQpResidual();
    setResidual(_sys, res, _var);

    if (_has_save_in)
      for (unsigned int i = 0; i < _save_in.size(); i++)
        _save_in[i]->sys().solution().set(_save_in[i]->nodalDofIndex(), res);
  }
}

void
NodalBC::computeJacobian()
{
  // We call the user's computeQpJacobian() function and store the
  // results in the _assembly object. We can't store them directly in
  // the element stiffness matrix, as they will only be inserted after
  // all the assembly is done.
  if (_var.isNodalDefined())
  {
    const Real cached_val = computeQpJacobian();
    const dof_id_type cached_row = _var.nodalDofIndex();

    // Cache the user's computeQpJacobian() value for later use.
    addJacobianElement(_fe_problem.assembly(0, _sys.number()),
                       cached_val,
                       cached_row,
                       cached_row,
                       /*scaling_factor=*/1);

    if (_has_diag_save_in)
      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().set(_diag_save_in[i]->nodalDofIndex(), cached_val);
  }
}

void
NodalBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    if (!_var.isNodalDefined())
      return;

    const Real cached_val = computeQpOffDiagJacobian(jvar_num);

    if (cached_val == 0.)
      // there's no reason to cache this if it's zero, and it can even lead to new nonzero
      // allocations
      return;

    const dof_id_type cached_row = _var.nodalDofIndex();
    // Note: this only works for Lagrange variables...
    const dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar_num, 0);

    // Cache the user's computeQpJacobian() value for later use.
    addJacobianElement(_fe_problem.assembly(0, _sys.number()),
                       cached_val,
                       cached_row,
                       cached_col,
                       /*scaling_factor=*/1);
  }
}

Real
NodalBC::computeQpJacobian()
{
  return 1.;
}

Real
NodalBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}

void
NodalBC::computeResidualAndJacobian()
{
  computeResidual();

  for (const auto & [ivariable, jvariable] : _fe_problem.couplingEntries(_tid))
  {
    const unsigned int ivar = ivariable->number();
    const unsigned int jvar = jvariable->number();

    if (ivar != _var.number())
      continue;

    if (_is_implicit)
      computeOffDiagJacobian(jvar);
  }

  /// TODO: add nonlocal Jacobians and scalar Jacobians
}
