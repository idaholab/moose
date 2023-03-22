//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "InputParameters.h"
#include "MooseArray.h"
#include "DualRealOps.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

InputParameters
LMKernel::validParams()
{
  auto params = ADKernelValue::validParams();
  params.addRequiredCoupledVar("lm_variable", "The lagrange multiplier variable");
  params.addParam<bool>(
      "lm_sign_positive",
      true,
      "Whether to use a positive sign when adding this object's residual to the Lagrange "
      "multiplier constraint equation. Positive or negative sign should be chosen such that the "
      "diagonals for the LM block of the matrix are positive");
  return params;
}

LMKernel::LMKernel(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _lm_var(*getVar("lm_variable", 0)),
    _lm(adCoupledValue("lm_variable")),
    _lm_test(_lm_var.phi()),
    _lm_sign(getParam<bool>("lm_sign_positive") ? 1. : -1)
{
}

void
LMKernel::computeResidual()
{
  std::vector<Real> strong_residuals(_qrule->n_points());

  precalculateResidual();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    strong_residuals[_qp] =
        MetaPhysicL::raw_value(this->precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp]);

  // Primal residual
  prepareVectorTag(_assembly, _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    for (_i = 0; _i < _test.size(); _i++)
      _local_re(_i) += _test[_i][_qp] * strong_residuals[_qp];

  accumulateTaggedLocalResidual();

  // LM residual
  prepareVectorTag(_assembly, _lm_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    for (_i = 0; _i < _lm_test.size(); _i++)
      _local_re(_i) += _lm_sign * _lm_test[_i][_qp] * strong_residuals[_qp];

  accumulateTaggedLocalResidual();
}

void
LMKernel::computeResidualsForJacobian()
{
  std::vector<ADReal> strong_residuals(_qrule->n_points());
  const auto resid_size = _test.size() + _lm_test.size();
  mooseAssert(resid_size == (_var.dofIndices().size() + _lm_var.dofIndices().size()),
              "The test function sizes and the dof indices sizes should match.");

  if (_residuals.size() != resid_size)
    _residuals.resize(resid_size, 0);

  // Resizing a std:vector doesn't zero everything
  for (auto & r : _residuals)
    r = 0;

  _all_dof_indices = _var.dofIndices();
  _all_dof_indices.insert(
      _all_dof_indices.end(), _lm_var.dofIndices().begin(), _lm_var.dofIndices().end());

  precalculateResidual();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    strong_residuals[_qp] = this->precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];

  // Primal
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    for (_i = 0; _i < _test.size(); _i++)
      _residuals[_i] += strong_residuals[_qp] * _test[_i][_qp];

  // LM
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    for (_i = 0; _i < _lm_test.size(); _i++)
      _residuals[_test.size() + _i] += _lm_sign * strong_residuals[_qp] * _lm_test[_i][_qp];
}
