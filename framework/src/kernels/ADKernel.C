//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "NonlinearSystemBase.h"

// libmesh includes
#include "libmesh/threads.h"

defineADBaseValidParams(ADKernel, KernelBase, params.registerBase("ADKernel"););

template <ComputeStage compute_stage>
ADKernel<compute_stage>::ADKernel(const InputParameters & parameters)
  : KernelBase(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(*mooseVariable()),
    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _u(_var.adSln<compute_stage>()),
    _grad_u(_var.adGradSln<compute_stage>())
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

template <ComputeStage compute_stage>
ADKernel<compute_stage>::~ADKernel()
{
}

template <ComputeStage compute_stage>
void
ADKernel<compute_stage>::computeResidual()
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
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

template <>
void
ADKernel<JACOBIAN>::computeResidual()
{
}

template <ComputeStage compute_stage>
void
ADKernel<compute_stage>::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  size_t ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
  {
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      DualReal residual =
          computeQpResidual(); // This will also compute the derivative with respect to all dofs
      for (_j = 0; _j < _var.phiSize(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * residual.derivatives()[ad_offset + _j];
    }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

template <>
void
ADKernel<RESIDUAL>::computeJacobian()
{
}

template <ComputeStage compute_stage>
void
ADKernel<compute_stage>::computeOffDiagJacobian(MooseVariableFEBase & jvar)
{
  auto jvar_num = jvar.number();

  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    size_t ad_offset = jvar_num * _sys.getMaxVarNDofsPerElem();

    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    if (_local_ke.m() != _test.size() || _local_ke.n() != jvar.phiSize())
      return;

    precalculateResidual();
    for (_i = 0; _i < _test.size(); _i++)
    {
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        DualReal residual =
            computeQpResidual(); // This will also compute the derivative with respect to all dofs

        for (_j = 0; _j < jvar.phiSize(); _j++)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * residual.derivatives()[ad_offset + _j];
      }
    }

    accumulateTaggedLocalMatrix();
  }
}

template <>
void
ADKernel<RESIDUAL>::computeOffDiagJacobian(MooseVariableFEBase &)
{
}

template <ComputeStage compute_stage>
void
ADKernel<compute_stage>::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
  /*
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
  */
}

// explicit instantiation is required for AD base classes
adBaseClass(ADKernel);
