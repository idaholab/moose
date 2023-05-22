//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMortarScalarBase.h"

// MOOSE includes
#include "Assembly.h"
#include "SystemBase.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "ADUtils.h"

InputParameters
ADMortarScalarBase::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  // This parameter can get renamed in derived class to a more relevant variable name
  params.addCoupledVar("scalar_variable", "Primary coupled scalar variable");
  params.addParam<bool>("compute_scalar_residuals", true, "Whether to compute scalar residuals");
  return params;
}

ADMortarScalarBase::ADMortarScalarBase(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _use_scalar(isParamValid("scalar_variable") ? true : false),
    _compute_scalar_residuals(!_use_scalar ? false : getParam<bool>("compute_scalar_residuals")),
    _kappa_var_ptr(_use_scalar ? getScalarVar("scalar_variable", 0) : nullptr),
    _kappa_var(_use_scalar ? _kappa_var_ptr->number() : 0),
    _k_order(_use_scalar ? _kappa_var_ptr->order() : 0),
    _kappa(_use_scalar ? _kappa_var_ptr->adSln() : _ad_zero)
{
}

void
ADMortarScalarBase::computeResidual()
{
  ADMortarConstraint::computeResidual();

  if (!_compute_scalar_residuals)
    return;

  std::vector<Real> scalar_residuals(_k_order);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    initScalarQpResidual();
    for (_h = 0; _h < _k_order; _h++)
      scalar_residuals[_h] += _JxW_msm[_qp] * _coord[_qp] * raw_value(computeScalarQpResidual());
  }
  addResiduals(
      _assembly, scalar_residuals, _kappa_var_ptr->dofIndices(), _kappa_var_ptr->scalingFactor());
}

void
ADMortarScalarBase::computeJacobian()
{
  // d-_var-residual / d-_var and d-_var-residual / d-jvar
  ADMortarConstraint::computeJacobian();

  if (!_compute_scalar_residuals)
    return;

  std::vector<ADReal> scalar_residuals;
  scalar_residuals.resize(_k_order, 0);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    initScalarQpResidual();
    for (_h = 0; _h < _k_order; _h++)
      scalar_residuals[_h] += _JxW_msm[_qp] * _coord[_qp] * computeScalarQpResidual();
  }
  addResidualsAndJacobianWithoutConstraints(
      _assembly, scalar_residuals, _kappa_var_ptr->dofIndices(), _kappa_var_ptr->scalingFactor());
}
