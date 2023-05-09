//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KernelScalarBase.h"

#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"

#include "libmesh/quadrature.h"

InputParameters
KernelScalarBase::validParams()
{
  InputParameters params = Kernel::validParams();
  // This parameter can get renamed in derived class to a more relevant variable name
  params.addCoupledVar("scalar_variable", "Primary coupled scalar variable");
  params.addParam<bool>("compute_scalar_residuals", true, "Whether to compute scalar residuals");
  params.addParam<bool>(
      "compute_field_residuals", true, "Whether to compute residuals for the field variable.");
  return params;
}

KernelScalarBase::KernelScalarBase(const InputParameters & parameters)
  : Kernel(parameters),
    _use_scalar(isParamValid("scalar_variable") ? true : false),
    _compute_scalar_residuals(!_use_scalar ? false : getParam<bool>("compute_scalar_residuals")),
    _compute_field_residuals(getParam<bool>("compute_field_residuals")),
    _kappa_var_ptr(_use_scalar ? getScalarVar("scalar_variable", 0) : nullptr),
    _kappa_var(_use_scalar ? _kappa_var_ptr->number() : 0),
    _k_order(_use_scalar ? _kappa_var_ptr->order() : 0),
    _kappa(_use_scalar ? (_is_implicit ? _kappa_var_ptr->sln() : _kappa_var_ptr->slnOld()) : _zero)
{
}

void
KernelScalarBase::computeResidual()
{
  if (_compute_field_residuals)
    Kernel::computeResidual(); // compute and assemble regular variable contributions

  if (_compute_scalar_residuals)
    computeScalarResidual();
}

void
KernelScalarBase::computeScalarResidual()
{
  std::vector<Real> scalar_residuals(_k_order);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpResidual();
    for (_h = 0; _h < _k_order; _h++)
      scalar_residuals[_h] += _JxW[_qp] * _coord[_qp] * computeScalarQpResidual();
  }

  addResiduals(
      _assembly, scalar_residuals, _kappa_var_ptr->dofIndices(), _kappa_var_ptr->scalingFactor());
}

void
KernelScalarBase::computeJacobian()
{
  if (_compute_field_residuals)
    Kernel::computeJacobian();

  if (_compute_scalar_residuals)
    computeScalarJacobian();
}

void
KernelScalarBase::computeScalarJacobian()
{
  _local_ke.resize(_k_order, _k_order);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpJacobian(_kappa_var);
    for (_h = 0; _h < _k_order; _h++)
      for (_l = 0; _l < _k_order; _l++)
        _local_ke(_h, _l) += _JxW[_qp] * _coord[_qp] * computeScalarQpJacobian();
  }

  addJacobian(_assembly,
              _local_ke,
              _kappa_var_ptr->dofIndices(),
              _kappa_var_ptr->dofIndices(),
              _kappa_var_ptr->scalingFactor());
}

void
KernelScalarBase::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (_use_scalar)
  {
    if (jvar_num == variable().number()) // column for this kernel's variable
    {
      if (_compute_field_residuals)
        Kernel::computeJacobian(); // d-_var-residual / d-_var
      if (_compute_scalar_residuals)
        computeScalarOffDiagJacobian(jvar_num); // d-_kappa-residual / d-_var
    }
    else if (jvar_num == _kappa_var) // column for this kernel's scalar variable
      // handle these in computeOffDiagJacobianScalar
      return;
    else // some other column for regular variable
    {
      if (_compute_field_residuals)
        Kernel::computeOffDiagJacobian(jvar_num); // d-_var-residual / d-jvar
      if (_compute_scalar_residuals)
        computeScalarOffDiagJacobian(jvar_num); // d-_kappa-residual / d-jvar
    }
  }
  else
  {
    if (jvar_num == variable().number()) // column for this kernel's variable
    {
      if (_compute_field_residuals)
        Kernel::computeJacobian(); // d-_var-residual / d-_var
    }
    else // some other column for regular variable
    {
      if (_compute_field_residuals)
        Kernel::computeOffDiagJacobian(jvar_num); // d-_var-residual / d-jvar
    }
  }
}

void
KernelScalarBase::computeScalarOffDiagJacobian(const unsigned int jvar_num)
{
  const auto & jvar = getVariable(jvar_num);
  if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    // Get dofs and order of this variable; at least one will be _var
    // const auto & jv0 = static_cast<const MooseVariable &>(jvar);
    // const auto & loc_phi = jv0.phi();
    const auto jvar_size = jvar.phiSize();
    _local_ke.resize(_k_order, jvar_size);

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initScalarQpOffDiagJacobian(jvar);
      for (_h = 0; _h < _k_order; _h++)
        for (_j = 0; _j < jvar_size; _j++)
          _local_ke(_h, _j) += _JxW[_qp] * _coord[_qp] * computeScalarQpOffDiagJacobian(jvar_num);
    }
  }
  else if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
    mooseError("Array variable cannot be coupled into Kernel Scalar currently");
  else
    mooseError("Vector variable cannot be coupled into Kernel Scalar currently");

  addJacobian(_assembly,
              _local_ke,
              _kappa_var_ptr->dofIndices(),
              jvar.dofIndices(),
              _kappa_var_ptr->scalingFactor());
}

void
KernelScalarBase::computeOffDiagJacobianScalarLocal(const unsigned int svar_num)
{
  // Get dofs and order of this scalar; at least one will be _kappa_var
  const auto & svar = _sys.getScalarVariable(_tid, svar_num);
  const unsigned int s_order = svar.order();
  _local_ke.resize(_test.size(), s_order);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpJacobian(svar_num);
    for (_i = 0; _i < _test.size(); _i++)
      for (_l = 0; _l < s_order; _l++)
        _local_ke(_i, _l) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(svar_num);
  }

  addJacobian(_assembly, _local_ke, _var.dofIndices(), svar.dofIndices(), _var.scalingFactor());
}

void
KernelScalarBase::computeOffDiagJacobianScalar(const unsigned int svar_num)
{
  precalculateOffDiagJacobianScalar(svar_num);
  if (_use_scalar)
  {
    if (svar_num == variable().number()) // column for this kernel's variable
      // this kernel's variable is not a scalar
      return;
    else if (svar_num == _kappa_var) // column for this kernel's scalar variable
    {
      // Perform assembly using method in Kernel; works for simple cases but not general
      // Kernel::computeOffDiagJacobianScalar(svar_num); // d-_var-residual / d-_kappa
      // Perform assembly using local_ke like d-_kappa_var-residual / d-_var
      if (_compute_field_residuals)
        computeOffDiagJacobianScalarLocal(svar_num); // d-_var-residual / d-_kappa
      if (_compute_scalar_residuals)
        computeScalarJacobian(); // d-_kappa-residual / d-_kappa
    }
    else // some other column for scalar variable
    {
      // Perform assembly using method in Kernel; works for simple cases but not general
      // Kernel::computeOffDiagJacobianScalar(svar_num); // d-_var-residual / d-jvar
      // Perform assembly using local_ke like d-_kappa_var-residual / d-_var
      if (_compute_field_residuals)
        computeOffDiagJacobianScalarLocal(svar_num); // d-_var-residual / d-svar
      if (_compute_scalar_residuals)
        computeScalarOffDiagJacobianScalar(svar_num); // d-_kappa-residual / d-svar
    }
  }
  else if (_compute_field_residuals)
    Kernel::computeOffDiagJacobianScalar(svar_num); // d-_var-residual / d-svar
}

void
KernelScalarBase::computeScalarOffDiagJacobianScalar(const unsigned int svar_num)
{
  // Get dofs and order of this scalar; at least one will be _kappa_var
  const auto & svar = _sys.getScalarVariable(_tid, svar_num);
  const unsigned int s_order = svar.order();
  _local_ke.resize(_k_order, s_order);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpJacobian(svar_num);
    for (_h = 0; _h < _k_order; _h++)
      for (_l = 0; _l < s_order; _l++)
        _local_ke(_h, _l) +=
            _JxW[_qp] * _coord[_qp] * computeScalarQpOffDiagJacobianScalar(svar_num);
  }

  addJacobian(_assembly,
              _local_ke,
              _kappa_var_ptr->dofIndices(),
              svar.dofIndices(),
              _kappa_var_ptr->scalingFactor());
}
