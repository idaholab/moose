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
  params.addRequiredCoupledVar("scalar_variable", "Primary coupled scalar variable");
  return params;
}

KernelScalarBase::KernelScalarBase(const InputParameters & parameters)
  : Kernel(parameters),
    // _kappa_var(coupledScalar("scalar_variable")), _kappa(coupledScalarValue("scalar_variable"))
    _kappa_var(*getScalarVar("scalar_variable", 0)),
    _kappa(_is_implicit ? _kappa_var.sln() : _kappa_var.slnOld())
{
  // add some error checks here
}

void
KernelScalarBase::computeResidual()
{
  Kernel::computeResidual(); // compute and assemble regular variable contributions

  // prepareVectorTagLower(_assembly, _kappa_var.number());

  // for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  // {
  //   initScalarQpResidual();
  //   for (_i = 0; _i < _kappa_var.order(); _i++)
  //     _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeScalarQpResidual();
  // }
  DenseVector<Number> & re = _assembly.residualBlock(_kappa_var.number());
  // DenseVector<Number> & re = _assembly.residualBlock(_kappa_var);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpResidual();
    for (_i = 0; _i < re.size(); _i++)
      re(_i) += _JxW[_qp] * _coord[_qp] * computeScalarQpResidual();
  }

  // accumulateTaggedLocalResidual();
}

void
KernelScalarBase::computeJacobian()
{
  Kernel::computeJacobian();

  computeScalarJacobian();
}

void
KernelScalarBase::computeScalarJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_kappa_var.number(), _kappa_var.number());
  // DenseMatrix<Number> & ke = _assembly.jacobianBlock(_kappa_var, _kappa_var);

  if (ke.n() == 0 || ke.m() == 0)
    return;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpJacobian(_kappa_var.number());
    // initScalarQpJacobian(_kappa_var);
    for (_i = 0; _i < ke.m(); _i++)
      for (_j = 0; _j < ke.n(); _j++)
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeScalarQpJacobian();
  }

  // prepareMatrixTagLower(_assembly, ivar, jvar, type_tr);

  // if (_local_ke.n() == 0 || _local_ke.m() == 0)
  //   return;

  // for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  // {
  //   initScalarQpJacobian(_kappa_var.number());
  //   for (_i = 0; _i < test_space.size(); _i++)
  //     for (_j = 0; _j < loc_phi.size(); _j++)
  //       _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeScalarQpJacobian(type);
  // }

  // accumulateTaggedLocalMatrix();
}

void
KernelScalarBase::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == variable().number()) // column for this kernel's variable
  {
    Kernel::computeJacobian(); // d-_var-residual / d-_var
    computeScalarOffDiagJacobian(jvar_num); // d-_kappa-residual / d-_var
  }
  else if (jvar_num == _kappa_var.number()) // column for this kernel's scalar variable
  // else if (jvar_num == _kappa_var) // column for this kernel's scalar variable
  {
    // handle these in computeOffDiagJacobianScalar
    return;
  }
  else // some other column for regular variable
  {
    Kernel::computeOffDiagJacobian(jvar_num); // d-_var-residual / d-jvar
    computeScalarOffDiagJacobian(jvar_num); // d-_kappa-residual / d-jvar
  }
}

void
KernelScalarBase::computeScalarOffDiagJacobian(const unsigned int jvar_num)
{

  const auto & jvar = getVariable(jvar_num);
  // prepareMatrixTagLower(_assembly, _kappa_var.number(), jvar_num, type);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(_kappa_var.number(), jvar_num);
  // DenseMatrix<Number> & kne = _assembly.jacobianBlock(_kappa_var, jvar_num);
  if (kne.n() == 0 || kne.m() == 0)
    return;

  if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    const auto & jv0 = static_cast<const MooseVariable &>(jvar);
    const auto & loc_phi = jv0.phi();

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initScalarQpOffDiagJacobian(jvar);
      for (_i = 0; _i < kne.m(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
          kne(_i, _j) += _JxW[_qp] * _coord[_qp] * computeScalarQpOffDiagJacobian(jvar_num);
    }
  }
  else if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
    mooseError("Array variable cannot be coupled into Kernel Scalar currently");
  else
    mooseError("Vector variable cannot be coupled into Kernel Scalar currently");

  // accumulateTaggedLocalMatrix();
}

void
KernelScalarBase::computeOffDiagJacobianScalarLocal(const unsigned int jvar_num)
{

  // prepareMatrixTagLower(_assembly, _kappa_var.number(), jvar_num, type);
  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), _kappa_var.number());
  // DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), _kappa_var);
  if (ken.n() == 0 || ken.m() == 0)
    return;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpJacobian(jvar_num);
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < ken.n(); _j++)
        ken(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(jvar_num);
  }

  // accumulateTaggedLocalMatrix();
}

void
KernelScalarBase::computeOffDiagJacobianScalar(const unsigned int jvar_num)
{
  if (jvar_num == variable().number()) // column for this kernel's variable
  {
    // this kernel's variable is not a scalar
    return;
  }
  else if (jvar_num == _kappa_var.number()) // column for this kernel's scalar variable
  // else if (jvar_num == _kappa_var) // column for this kernel's scalar variable
  {
    Kernel::computeOffDiagJacobianScalar(jvar_num); // d-_var-residual / d-_kappa
    // computeOffDiagJacobianScalarLocal(jvar_num); // d-_var-residual / d-_kappa
    computeScalarJacobian(); // d-_kappa-residual / d-_kappa
  }
  else // some other column for scalar variable
  {
    Kernel::computeOffDiagJacobianScalar(jvar_num); // d-_var-residual / d-jvar
    computeScalarOffDiagJacobianScalar(jvar_num); // d-_kappa-residual / d-jvar
  }
}

void
KernelScalarBase::computeScalarOffDiagJacobianScalar(const unsigned int jvar)
{
  // prepareMatrixTagLower(_assembly, _kappa_var.number(), jvar);
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_kappa_var.number(), jvar);
  // DenseMatrix<Number> & ke = _assembly.jacobianBlock(_kappa_var, jvar);

  if (ke.n() == 0 || ke.m() == 0)
    return;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initScalarQpJacobian(jvar);
    for (_i = 0; _i < ke.m(); _i++)
      for (_j = 0; _j < ke.n(); _j++)
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeScalarQpOffDiagJacobianScalar(jvar);
  }

  // accumulateTaggedLocalMatrix();
}
