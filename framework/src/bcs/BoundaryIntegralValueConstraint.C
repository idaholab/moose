//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryIntegralValueConstraint.h"

#include "MooseVariableScalar.h"

#include <array>

registerMooseObject("MooseApp", BoundaryIntegralValueConstraint);

InputParameters
BoundaryIntegralValueConstraint::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Enforces a prescribed average value for a finite element variable on a boundary using a "
      "scalar Lagrange multiplier.");
  params.addParam<PostprocessorName>("phi0", "0", "The value that the constraint will enforce.");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier scalar variable");
  return params;
}

BoundaryIntegralValueConstraint::BoundaryIntegralValueConstraint(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _phi0(getPostprocessorValue("phi0")),
    _lambda_var(*getScalarVar("lambda", 0)),
    _lambda(coupledScalarValue("lambda"))
{
  if (_lambda_var.order() != 1)
    paramError("lambda", "The lambda variable must be a first-order scalar variable.");
}

std::set<std::string>
BoundaryIntegralValueConstraint::additionalROVariables()
{
  return {_lambda_var.name()};
}

Real
BoundaryIntegralValueConstraint::computeQpResidual()
{
  return _lambda[0] * _test[_i][_qp];
}

void
BoundaryIntegralValueConstraint::computeResidual()
{
  IntegratedBC::computeResidual();
  computeScalarResidual();
}

void
BoundaryIntegralValueConstraint::computeJacobian()
{
  // The field equation has no diagonal dependence from this constraint, but we still assemble the
  // zero block so the matrix structure is complete.
  IntegratedBC::computeJacobian();
  computeScalarJacobian();
}

void
BoundaryIntegralValueConstraint::computeOffDiagJacobian(const unsigned int jvar)
{
  if (jvar == _var.number())
  {
    computeJacobian();
    computeScalarFieldJacobian(jvar);
  }
  else
    IntegratedBC::computeOffDiagJacobian(jvar);
}

void
BoundaryIntegralValueConstraint::computeOffDiagJacobianScalar(const unsigned int jvar)
{
  if (jvar == _lambda_var.number())
    computeFieldScalarJacobian();
}

void
BoundaryIntegralValueConstraint::computeResidualAndJacobian()
{
  computeResidual();

  if (_is_implicit)
  {
    computeJacobian();
    // The residual-and-Jacobian-together path bypasses ComputeFullJacobianThread, so assemble the
    // scalar off-diagonal blocks here.
    computeFieldScalarJacobian();
    computeScalarFieldJacobian(_var.number());
  }
}

void
BoundaryIntegralValueConstraint::computeScalarResidual()
{
  mooseAssert(_lambda_var.dofIndices().size() == 1, "The lambda variable should have one dof");

  std::array<Real, 1> residual{{0.0}};

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    residual[0] += _JxW[_qp] * _coord[_qp] * (_u[_qp] - _phi0);

  addResiduals(_assembly, residual, _lambda_var.dofIndices(), _lambda_var.scalingFactor());
}

void
BoundaryIntegralValueConstraint::computeScalarJacobian()
{
  mooseAssert(_lambda_var.dofIndices().size() == 1, "The lambda variable should have one dof");

  DenseMatrix<Real> zero(1, 1);
  addJacobian(_assembly,
              zero,
              _lambda_var.dofIndices(),
              _lambda_var.dofIndices(),
              _lambda_var.scalingFactor());
}

void
BoundaryIntegralValueConstraint::computeFieldScalarJacobian()
{
  mooseAssert(_lambda_var.dofIndices().size() == 1, "The lambda variable should have one dof");

  DenseMatrix<Real> jacobian(_test.size(), 1);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      jacobian(_i, 0) += _JxW[_qp] * _coord[_qp] * _test[_i][_qp];

  addJacobian(
      _assembly, jacobian, _var.dofIndices(), _lambda_var.dofIndices(), _var.scalingFactor());
}

void
BoundaryIntegralValueConstraint::computeScalarFieldJacobian(const unsigned int jvar)
{
  if (jvar != _var.number())
    return;

  // _phi is bound to the assembly's _phi_face slot, which holds whichever variable was last
  // prepared. Re-prepare _var so _phi corresponds to _var.
  prepareShapes(jvar);

  mooseAssert(_lambda_var.dofIndices().size() == 1, "The lambda variable should have one dof");

  DenseMatrix<Real> jacobian(1, _phi.size());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_j = 0; _j < _phi.size(); _j++)
      jacobian(0, _j) += _JxW[_qp] * _coord[_qp] * _phi[_j][_qp];

  addJacobian(_assembly,
              jacobian,
              _lambda_var.dofIndices(),
              _var.dofIndices(),
              _lambda_var.scalingFactor());
}
