//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernelScalarBase.h"

#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "ADUtils.h"

#include "libmesh/quadrature.h"

InputParameters
ADKernelScalarBase::validParams()
{
  InputParameters params = ADKernel::validParams();
  // This parameter can get renamed in derived class to a more relevant variable name
  params.addCoupledVar("scalar_variable", "Primary coupled scalar variable");
  params.addParam<bool>("compute_scalar_residuals", true, "Whether to compute scalar residuals");
  params.addParam<bool>(
      "compute_field_residuals", true, "Whether to compute residuals for the field variable.");
  return params;
}

ADKernelScalarBase::ADKernelScalarBase(const InputParameters & parameters)
  : ADKernel(parameters),
    _use_scalar(isParamValid("scalar_variable") ? true : false),
    _compute_scalar_residuals(!_use_scalar ? false : getParam<bool>("compute_scalar_residuals")),
    _compute_field_residuals(getParam<bool>("compute_field_residuals")),
    _kappa_var_ptr(_use_scalar ? getScalarVar("scalar_variable", 0) : nullptr),
    _kappa_var(_use_scalar ? _kappa_var_ptr->number() : 0),
    _k_order(_use_scalar ? _kappa_var_ptr->order() : 0),
    _kappa(_use_scalar ? _kappa_var_ptr->adSln() : _ad_zero)
{
}

void
ADKernelScalarBase::computeResidual()
{
  if (_compute_field_residuals)
    ADKernel::computeResidual(); // compute and assemble regular variable contributions

  if (_compute_scalar_residuals)
  {
    std::vector<Real> scalar_residuals(_k_order);
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initScalarQpResidual();
      for (_h = 0; _h < _k_order; _h++)
        scalar_residuals[_h] += _JxW[_qp] * _coord[_qp] * raw_value(computeScalarQpResidual());
    }
    _assembly.processResiduals(scalar_residuals,
                               _kappa_var_ptr->dofIndices(),
                               _vector_tags,
                               _kappa_var_ptr->scalingFactor());
  }
}

void
ADKernelScalarBase::computeJacobian()
{
  if (_compute_field_residuals)
    ADKernel::computeJacobian();

  if (_compute_scalar_residuals)
  {
    computeScalarResidualsForJacobian();
    _assembly.processResidualsAndJacobian(_scalar_residuals,
                                          _kappa_var_ptr->dofIndices(),
                                          _vector_tags,
                                          _matrix_tags,
                                          _kappa_var_ptr->scalingFactor());
  }
}

void
ADKernelScalarBase::computeOffDiagJacobian(const unsigned int jvar_num)
{
  // Only need to do this once because AD does all the derivatives at once
  if (jvar_num == _var.number())
    computeJacobian();
}

void
ADKernelScalarBase::computeOffDiagJacobianScalar(const unsigned int /*jvar_num*/)
{
}

void
ADKernelScalarBase::computeResidualAndJacobian()
{
  if (_compute_field_residuals)
    ADKernel::computeResidualAndJacobian();

  if (_compute_scalar_residuals)
  {
    computeScalarResidualsForJacobian();
    _assembly.processResidualsAndJacobian(_scalar_residuals,
                                          _kappa_var_ptr->dofIndices(),
                                          _vector_tags,
                                          _matrix_tags,
                                          _kappa_var_ptr->scalingFactor());
  }
}

void
ADKernelScalarBase::computeScalarResidualsForJacobian()
{
  if (_scalar_residuals.size() != _k_order)
    _scalar_residuals.resize(_k_order, 0);
  for (auto & sr : _scalar_residuals)
    sr = 0;

  // precalculateResidual was already run for the field variable
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_h = 0; _h < _k_order; _h++)
      _scalar_residuals[_h] += _JxW[_qp] * _coord[_qp] * computeScalarQpResidual();
}
