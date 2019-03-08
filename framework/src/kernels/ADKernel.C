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

defineADBaseValidParams(ADKernel, KernelBase, params.registerBase("Kernel"););
defineADBaseValidParams(ADVectorKernel, KernelBase, params.registerBase("VectorKernel"););

template <typename T, ComputeStage compute_stage>
ADKernelTempl<T, compute_stage>::ADKernelTempl(const InputParameters & parameters)
  : KernelBase(parameters),
    MooseVariableInterface<T>(this,
                              false,
                              "variable",
                              Moose::VarKindType::VAR_NONLINEAR,
                              std::is_same<T, Real>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                           : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*this->mooseVariable()),
    _test(_var.phi()),
    _grad_test(_var.template adGradPhi<compute_stage>()),
    _u(_var.template adSln<compute_stage>()),
    _grad_u(_var.template adGradSln<compute_stage>()),
    _ad_JxW(_assembly.adJxW<compute_stage>()),
    _ad_coord(_assembly.adCoordTransformation<compute_stage>()),
    _ad_q_point(_assembly.template adQPoints<compute_stage>()),
    _phi(_assembly.phi(_var)),
    _grad_phi(_assembly.template adGradPhi<T, compute_stage>(_var))
{
  addMooseVariableDependency(this->mooseVariable());
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

template <typename T, ComputeStage compute_stage>
void
ADKernelTempl<T, compute_stage>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual();

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
ADKernelTempl<Real, JACOBIAN>::computeResidual()
{
}

template <>
void
ADKernelTempl<RealVectorValue, JACOBIAN>::computeResidual()
{
}

template <typename T, ComputeStage compute_stage>
void
ADKernelTempl<T, compute_stage>::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  size_t ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
  {
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      DualReal residual = _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual();
      for (_j = 0; _j < _var.phiSize(); _j++)
        _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
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
ADKernelTempl<Real, RESIDUAL>::computeJacobian()
{
}
template <>
void
ADKernelTempl<RealVectorValue, RESIDUAL>::computeJacobian()
{
}

template <typename T, ComputeStage compute_stage>
void
ADKernelTempl<T, compute_stage>::computeADOffDiagJacobian()
{
  std::vector<DualReal> residuals(_test.size(), 0);

  precalculateResidual();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      residuals[_i] += _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual();

  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> & ce =
      _assembly.couplingEntries();
  for (const auto & it : ce)
  {
    MooseVariableFEBase & ivariable = *(it.first);
    MooseVariableFEBase & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivar != _var.number())
      continue;

    size_t ad_offset = jvar * _sys.getMaxVarNDofsPerElem();

    prepareMatrixTag(_assembly, ivar, jvar);

    if (_local_ke.m() != _test.size() || _local_ke.n() != jvariable.phiSize())
      continue;

    precalculateResidual();
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jvariable.phiSize(); _j++)
        _local_ke(_i, _j) += residuals[_i].derivatives()[ad_offset + _j];

    accumulateTaggedLocalMatrix();
  }
}

template <typename T, ComputeStage compute_stage>
void
ADKernelTempl<T, compute_stage>::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
}

template class ADKernelTempl<Real, RESIDUAL>;
template class ADKernelTempl<Real, JACOBIAN>;
template class ADKernelTempl<RealVectorValue, RESIDUAL>;
template class ADKernelTempl<RealVectorValue, JACOBIAN>;
