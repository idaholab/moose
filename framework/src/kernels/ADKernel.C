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
#include "ADUtils.h"

// libmesh includes
#include "libmesh/threads.h"

template <typename T>
InputParameters
ADKernelTempl<T>::validParams()
{
  auto params = KernelBase::validParams();
  if (std::is_same<T, Real>::value)
    params.registerBase("Kernel");
  else if (std::is_same<T, RealVectorValue>::value)
    params.registerBase("VectorKernel");
  else
    ::mooseError("unsupported ADKernelTempl specialization");
  return params;
}

template <typename T>
ADKernelTempl<T>::ADKernelTempl(const InputParameters & parameters)
  : KernelBase(parameters),
    MooseVariableInterface<T>(this,
                              false,
                              "variable",
                              Moose::VarKindType::VAR_NONLINEAR,
                              std::is_same<T, Real>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                           : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*this->mooseVariable()),
    _test(_var.phi()),
    _grad_test(_var.adGradPhi()),
    _regular_grad_test(_var.gradPhi()),
    _u(_var.adSln()),
    _grad_u(_var.adGradSln()),
    _ad_JxW(_assembly.adJxW()),
    _ad_coord(_assembly.adCoordTransformation()),
    _ad_q_point(_assembly.adQPoints()),
    _phi(_assembly.phi(_var)),
    _grad_phi(_assembly.template adGradPhi<T>(_var)),
    _regular_grad_phi(_assembly.gradPhi(_var)),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _my_elem(nullptr)
{
  _subproblem.haveADObjects(true);

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

  if (_use_displaced_mesh && _displacements.empty())
    mooseError("ADKernel ",
               name(),
               "has been asked to act on the displaced mesh, but no displacements have been "
               "coupled in. Your Jacobian will be wrong without that coupling");
}

template <typename T>
void
ADKernelTempl<T>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
        _local_re(_i) += raw_value(_ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual());
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
        _local_re(_i) += raw_value(_JxW[_qp] * _coord[_qp] * computeQpResidual());

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

template <typename T>
void
ADKernelTempl<T>::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  auto ad_offset =
      Moose::adOffset(_var.number(), _sys.getMaxVarNDofsPerElem(), Moose::ElementType::Element);

  precalculateResidual();

  if (_use_displaced_mesh)
    for (_i = 0; _i < _test.size(); _i++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        DualReal residual = _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual();
        for (_j = 0; _j < _var.phiSize(); _j++)
          _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
      }
  else
    for (_i = 0; _i < _test.size(); _i++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        DualReal residual = _JxW[_qp] * _coord[_qp] * computeQpResidual();
        for (_j = 0; _j < _var.phiSize(); _j++)
          _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
      }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && !_sys.computingScalingJacobian())
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

template <typename T>
void
ADKernelTempl<T>::jacobianSetup()
{
  _my_elem = nullptr;
}

template <typename T>
void
ADKernelTempl<T>::computeOffDiagJacobian(MooseVariableFEBase &)
{
  if (_my_elem != _current_elem)
  {
    computeADOffDiagJacobian();
    _my_elem = _current_elem;
  }
}

template <typename T>
void
ADKernelTempl<T>::computeADOffDiagJacobian()
{
  if (_residuals.size() != _test.size())
    _residuals.resize(_test.size(), 0);
  for (auto & r : _residuals)
    r = 0;

  precalculateResidual();
  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      _r = _ad_JxW[_qp];
      _r *= _ad_coord[_qp];
      for (_i = 0; _i < _test.size(); _i++)
        _residuals[_i] += _r * computeQpResidual();
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < _test.size(); _i++)
        _residuals[_i] += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  auto & ce = _assembly.couplingEntries();
  for (const auto & it : ce)
  {
    MooseVariableFEBase & ivariable = *(it.first);
    MooseVariableFEBase & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    // If ivar isn't this->_var, then continue
    // Also we don't currently support coupling with FV variables
    if (ivar != _var.number() || jvariable.isFV())
      continue;

    auto ad_offset =
        Moose::adOffset(jvar, _sys.getMaxVarNDofsPerElem(), Moose::ElementType::Element);

    prepareMatrixTag(_assembly, ivar, jvar);

    if (_local_ke.m() != _test.size() || _local_ke.n() != jvariable.phiSize())
      continue;

    precalculateResidual();
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jvariable.phiSize(); _j++)
        _local_ke(_i, _j) += _residuals[_i].derivatives()[ad_offset + _j];

    accumulateTaggedLocalMatrix();
  }
}

template <typename T>
void
ADKernelTempl<T>::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
}

template class ADKernelTempl<Real>;
template class ADKernelTempl<RealVectorValue>;
