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
  params.addParam<VariableName>("scalar_variable", "Primary coupled scalar variable");
  // This name is fixed and required to be equal to the previous parameter; need to add error
  // checks...
  params.addCoupledVar("coupled_scalar", "Repeat name of scalar variable to ensure dependency");
  return params;
}

KernelScalarBase::KernelScalarBase(const InputParameters & parameters)
  : Kernel(parameters),
    _use_scalar(isParamValid("scalar_variable") ? true : false),
    _kappa_dummy(),
    _kappa_var_ptr(
        _use_scalar ? &_sys.getScalarVariable(_tid, parameters.get<VariableName>("scalar_variable"))
                    : nullptr),
    _kappa_var(_use_scalar ? _kappa_var_ptr->number() : 0),
    _k_order(_use_scalar ? _kappa_var_ptr->order() : 0),
    _kappa(_use_scalar ? (_is_implicit ? _kappa_var_ptr->sln() : _kappa_var_ptr->slnOld())
                       : _kappa_dummy)
{
  // add some error checks here
}

void
KernelScalarBase::computeResidual()
{
  Kernel::computeResidual(); // compute and assemble regular variable contributions

  if (_use_scalar)
  {
    std::vector<Real> scalar_residuals(_k_order);

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initScalarQpResidual();
      for (_h = 0; _h < _k_order; _h++)
        scalar_residuals[_h] += _JxW[_qp] * _coord[_qp] * computeScalarQpResidual();
    }

    _assembly.processResiduals(scalar_residuals,
                               _kappa_var_ptr->dofIndices(),
                               _vector_tags,
                               _kappa_var_ptr->scalingFactor());
  }
}

void
KernelScalarBase::computeJacobian()
{
  Kernel::computeJacobian();

  if (_use_scalar)
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

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _kappa_var_ptr->dofIndices(),
                                 _kappa_var_ptr->dofIndices(),
                                 _kappa_var_ptr->scalingFactor(),
                                 matrix_tag);
}

void
KernelScalarBase::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (_use_scalar)
  {
    if (jvar_num == variable().number()) // column for this kernel's variable
    {
      Kernel::computeJacobian();              // d-_var-residual / d-_var
      computeScalarOffDiagJacobian(jvar_num); // d-_kappa-residual / d-_var
    }
    else if (jvar_num == _kappa_var) // column for this kernel's scalar variable
    {
      // handle these in computeOffDiagJacobianScalar
      return;
    }
    else // some other column for regular variable
    {
      Kernel::computeOffDiagJacobian(jvar_num); // d-_var-residual / d-jvar
      computeScalarOffDiagJacobian(jvar_num);   // d-_kappa-residual / d-jvar
    }
  }
  else
  {
    if (jvar_num == variable().number()) // column for this kernel's variable
    {
      Kernel::computeJacobian(); // d-_var-residual / d-_var
    }
    else // some other column for regular variable
    {
      Kernel::computeOffDiagJacobian(jvar_num); // d-_var-residual / d-jvar
    }
  }
}

void
KernelScalarBase::computeScalarOffDiagJacobian(const unsigned int jvar_num)
{

  const auto & jvar = getVariable(jvar_num);
  // Assumes all coupling variables have same test functions as var/primary/secondary
  // _local_ke.resize(_k_order, _test.size());
  _local_ke.resize(_k_order, jvar.phiSize());

  if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    const auto & jv0 = static_cast<const MooseVariable &>(jvar);
    const auto & loc_phi = jv0.phi();

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initScalarQpOffDiagJacobian(jvar);
      for (_h = 0; _h < _k_order; _h++)
        for (_j = 0; _j < loc_phi.size(); _j++)
          _local_ke(_h, _j) += _JxW[_qp] * _coord[_qp] * computeScalarQpOffDiagJacobian(jvar_num);
    }
  }
  else if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
    mooseError("Array variable cannot be coupled into Kernel Scalar currently");
  else
    mooseError("Vector variable cannot be coupled into Kernel Scalar currently");

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _kappa_var_ptr->dofIndices(),
                                 jvar.dofIndices(),
                                 _kappa_var_ptr->scalingFactor(),
                                 matrix_tag);
}

void
KernelScalarBase::computeOffDiagJacobianScalarLocal(const unsigned int jvar_num)
{

  // prepareMatrixTagLower(_assembly, _kappa_var.number(), jvar_num, type);
  // DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), _kappa_var);
  // if (ken.n() == 0 || ken.m() == 0)
  //   return;
  _local_ke.resize(_test.size(), _k_order);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpJacobian(jvar_num);
    for (_i = 0; _i < _test.size(); _i++)
      for (_l = 0; _l < _k_order; _l++)
        // ken(_i, _l) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(jvar_num);
        _local_ke(_i, _l) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(jvar_num);
  }

  // accumulateTaggedLocalMatrix();
  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _var.dofIndices(),
                                 _kappa_var_ptr->dofIndices(),
                                 _var.scalingFactor(),
                                 matrix_tag);
}

void
KernelScalarBase::computeOffDiagJacobianScalar(const unsigned int jvar_num)
{
  if (_use_scalar)
  {
    if (jvar_num == variable().number()) // column for this kernel's variable
    {
      // this kernel's variable is not a scalar
      return;
    }
    else if (jvar_num == _kappa_var) // column for this kernel's scalar variable
    {
      // Perform assembly using method in Kernel
      // Kernel::computeOffDiagJacobianScalar(jvar_num); // d-_var-residual / d-_kappa
      // Perform assembly using DenseMatrix like d-_kappa_var-residual / d-_var
      computeOffDiagJacobianScalarLocal(jvar_num); // d-_var-residual / d-_kappa
      computeScalarJacobian();                     // d-_kappa-residual / d-_kappa
    }
    else // some other column for scalar variable
    {
      Kernel::computeOffDiagJacobianScalar(jvar_num); // d-_var-residual / d-jvar
      computeScalarOffDiagJacobianScalar(jvar_num);   // d-_kappa-residual / d-jvar
    }
  }
  else
    Kernel::computeOffDiagJacobianScalar(jvar_num); // d-_var-residual / d-jvar
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
    initScalarQpJacobian(_kappa_var);
    for (_h = 0; _h < _k_order; _h++)
      for (_l = 0; _l < s_order; _l++)
        _local_ke(_h, _l) +=
            _JxW[_qp] * _coord[_qp] * computeScalarQpOffDiagJacobianScalar(svar_num);
  }

  for (const auto & matrix_tag : _matrix_tags)
    _assembly.cacheJacobianBlock(_local_ke,
                                 _kappa_var_ptr->dofIndices(),
                                 svar.dofIndices(),
                                 _kappa_var_ptr->scalingFactor(),
                                 matrix_tag);
}
