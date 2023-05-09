//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVScalarLagrangeMultiplierConstraint.h"

#include "MooseVariableScalar.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

InputParameters
FVScalarLagrangeMultiplierConstraint::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Base class for imposing constraints using scalar Lagrange multipliers");
  params.addParam<PostprocessorName>("phi0", "0", "The value that the constraint will enforce.");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier variable");
  return params;
}

FVScalarLagrangeMultiplierConstraint::FVScalarLagrangeMultiplierConstraint(
    const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _phi0(getPostprocessorValue("phi0")),
    _lambda_var(*getScalarVar("lambda", 0)),
    _lambda(adCoupledScalarValue("lambda"))
{
}

void
FVScalarLagrangeMultiplierConstraint::computeResidualAndJacobian()
{
  const auto volume = _assembly.elemVolume();
  _assembly.processResidualAndJacobian(
      _lambda[0] * volume, _var.dofIndices()[0], _vector_tags, _matrix_tags);
  _assembly.processResidualAndJacobian(
      computeQpResidual() * volume, _lambda_var.dofIndices()[0], _vector_tags, _matrix_tags);
}

void
FVScalarLagrangeMultiplierConstraint::computeResidual()
{
  // Primal residual
  prepareVectorTag(_assembly, _var.number());
  mooseAssert(_local_re.size() == 1, "We should only have a single dof");
  mooseAssert(_lambda.size() == 1 && _lambda_var.order() == 1,
              "The lambda variable should be first order");
  _local_re(0) += MetaPhysicL::raw_value(_lambda[0]) * _assembly.elemVolume();
  accumulateTaggedLocalResidual();

  // LM residual. We may not have any actual ScalarKernels in our simulation so we need to manually
  // make sure the scalar residuals get cached for later addition
  const auto lm_r = MetaPhysicL::raw_value(computeQpResidual()) * _assembly.elemVolume();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have a single dof");
  _assembly.processResidual(lm_r, _lambda_var.dofIndices()[0], _vector_tags);
}

void
FVScalarLagrangeMultiplierConstraint::computeJacobian()
{
}

void
FVScalarLagrangeMultiplierConstraint::computeOffDiagJacobian()
{
  // Primal
  mooseAssert(_lambda.size() == 1 && _lambda_var.order() == 1,
              "The lambda variable should be first order");
  const auto primal_r = _lambda[0] * _assembly.elemVolume();
  mooseAssert(_var.dofIndices().size() == 1, "We should only have one dof");
  _assembly.processJacobian(primal_r, _var.dofIndices()[0], _matrix_tags);

  // LM
  const auto lm_r = computeQpResidual() * _assembly.elemVolume();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have one dof");
  _assembly.processJacobian(lm_r, _lambda_var.dofIndices()[0], _matrix_tags);
}
