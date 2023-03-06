//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KernelValue.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "libmesh/quadrature.h"

InputParameters
KernelValue::validParams()
{
  return Kernel::validParams();
}

KernelValue::KernelValue(const InputParameters & parameters) : Kernel(parameters) {}

void
KernelValue::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  const unsigned int n_test = _test.size();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    Real value = precomputeQpResidual() * _JxW[_qp] * _coord[_qp];
    for (_i = 0; _i < n_test; _i++) // target for auto vectorization
      _local_re(_i) += value * _test[_i][_qp];
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
  }
}

void
KernelValue::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  const unsigned int n_test = _test.size();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_j = 0; _j < _phi.size(); _j++)
    {
      Real value = precomputeQpJacobian() * _JxW[_qp] * _coord[_qp];
      for (_i = 0; _i < n_test; _i++) // target for auto vectorization
        _local_ke(_i, _j) += value * _test[_i][_qp];
    }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++) // target for auto vectorization
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

void
KernelValue::computeOffDiagJacobian(const unsigned int jvar_num)
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

    for (_j = 0; _j < phi_size; _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        for (_i = 0; _i < _test.size(); _i++)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar_num);
    accumulateTaggedLocalMatrix();
  }
}

Real
KernelValue::computeQpResidual()
{
  mooseError("Override precomputeQpResidual() in your KernelValue derived class!");
}

Real
KernelValue::precomputeQpJacobian()
{
  return 0.0;
}
