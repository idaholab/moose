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
  params.addParam<VariableName>("scalar_variable", "Primary coupled scalar variable");
  // This name is fixed and required to be equal to the previous parameter; need to add error
  // checks...
  params.addCoupledVar("coupled_scalar", "Repeat name of scalar variable to ensure dependency");
  return params;
}

ADKernelScalarBase::ADKernelScalarBase(const InputParameters & parameters)
  : ADKernel(parameters),
    _use_scalar(isParamValid("scalar_variable") ? true : false),
    _kappa_dummy(),
    _kappa_var_ptr(
        _use_scalar ? &_sys.getScalarVariable(_tid, parameters.get<VariableName>("scalar_variable"))
                    : nullptr),
    _kappa_var(_use_scalar ? _kappa_var_ptr->number() : 0),
    _k_order(_use_scalar ? _kappa_var_ptr->order() : 0),
    _kappa(_use_scalar ? _kappa_var_ptr->adSln() : _kappa_dummy)
{
  // add some error checks here
}

void
ADKernelScalarBase::computeResidual()
{
  ADKernel::computeResidual(); // compute and assemble regular variable contributions

  if (_use_scalar)
  {
    std::vector<Real> scalar_residuals(_k_order);
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initScalarQpResidual();
      for (_h = 0; _h < _k_order; _h++)
      {
        scalar_residuals[_h] += _JxW[_qp] * _coord[_qp] * raw_value(computeScalarQpResidual());
      }
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
  ADKernel::computeJacobian();

  if (_use_scalar)
  {
#ifndef MOOSE_GLOBAL_AD_INDEXING
    mooseError("Jacobian assembly not coded for non-default AD");
#endif
  }
}

void
ADKernelScalarBase::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (_use_scalar)
  {
#ifndef MOOSE_GLOBAL_AD_INDEXING
    mooseError("off-diagonal Jacobian assembly not coded for non-default AD");
#endif
  }
  else
  {
    ADKernel::computeOffDiagJacobian(jvar_num); // d-_var-residual / d-jvar
  }
}

void
ADKernelScalarBase::computeOffDiagJacobianScalar(const unsigned int jvar_num)
{
  if (_use_scalar)
  {
#ifndef MOOSE_GLOBAL_AD_INDEXING
    mooseError("off-diagonal Jacobian assembly not coded for non-default AD");
#endif
  }
  else
    ADKernel::computeOffDiagJacobianScalar(jvar_num); // d-_var-residual / d-jvar
}

void
ADKernelScalarBase::computeResidualAndJacobian()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  ADKernel::computeResidualAndJacobian();

  if (_use_scalar)
  {
    computeScalarResidualsForJacobian();
    _assembly.processResidualsAndJacobian(_scalar_residuals,
                                          _kappa_var_ptr->dofIndices(),
                                          _vector_tags,
                                          _matrix_tags,
                                          _kappa_var_ptr->scalingFactor());
  }
#else
  mooseError("residual and jacobian together only supported for global AD indexing");
#endif
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
