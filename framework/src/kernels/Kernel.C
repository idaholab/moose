//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Kernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

InputParameters
Kernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("Kernel");
  return params;
}

Kernel::Kernel(const InputParameters & parameters)
  : KernelBase(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(*mooseVariable()),
    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _phi(_assembly.phi(_var)),
    _grad_phi(_assembly.gradPhi(_var)),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld())
{
  addMooseVariableDependency(mooseVariable());
  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getStandardVariable(_tid, _save_in_strings[i]);

    if (_fe_problem.getNonlinearSystemBase().hasVariable(_save_in_strings[i]))
      paramError("save_in", "cannot use solution variable as save-in variable");

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

    if (_fe_problem.getNonlinearSystemBase().hasVariable(_diag_save_in_strings[i]))
      paramError("diag_save_in", "cannot use solution variable as diag save-in variable");

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
Kernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
  }
}

void
Kernel::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  precalculateJacobian();
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && !_sys.computingScalingJacobian())
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

void
Kernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  const auto & jvar = getVariable(jvar_num);

  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting
    // on the displaced mesh
    auto phi_size = jvar.dofIndices().size();
    mooseAssert(
        phi_size * jvar.count() == _local_ke.n(),
        "The size of the phi container does not match the number of local Jacobian columns");

    if (_local_ke.m() != _test.size())
      return;

    precalculateOffDiagJacobian(jvar_num);
    if (jvar.count() == 1)
    {
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < phi_size; _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar.number());
    }
    else
    {
      unsigned int n = phi_size;
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < n; _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          {
            RealEigenVector v = _JxW[_qp] * _coord[_qp] *
                                computeQpOffDiagJacobianArray(static_cast<ArrayMooseVariable &>(
                                    const_cast<MooseVariableFieldBase &>(jvar)));
            for (unsigned int k = 0; k < v.size(); ++k)
              _local_ke(_i, _j + k * n) += v(k);
          }
    }

    accumulateTaggedLocalMatrix();
  }
}

void
Kernel::computeOffDiagJacobianScalar(const unsigned int jvar)
{
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  prepareMatrixTag(_assembly, _var.number(), jvar);

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(jvar);

  accumulateTaggedLocalMatrix();
}

void
Kernel::computeResidualAndJacobian()
{
  computeResidual();

  for (const auto & [ivariable, jvariable] : _fe_problem.couplingEntries(_tid))
  {
    const unsigned int ivar = ivariable->number();
    const unsigned int jvar = jvariable->number();

    if (ivar != _var.number())
      continue;

    if (_is_implicit)
    {
      prepareShapes(jvar);
      computeOffDiagJacobian(jvar);
    }
  }

  /// TODO: add nonlocal Jacobians and scalar Jacobians
}
