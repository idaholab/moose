//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVScalarLagrangeMultiplierInterface.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"

InputParameters
FVScalarLagrangeMultiplierInterface::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addClassDescription(
      "This class should be inherited to create interface penalties in finite volume.");
  params.addRequiredCoupledVar("lambda", "The name of the scalar lagrange multiplier");

  return params;
}

FVScalarLagrangeMultiplierInterface::FVScalarLagrangeMultiplierInterface(
    const InputParameters & params)
  : FVInterfaceKernel(params),
    _lambda_var(*getScalarVar("lambda", 0)),
    _lambda(adCoupledScalarValue("lambda"))
{
  if (var1().sys().number() != var2().sys().number())
    mooseError(this->type(), " does not support multiple nonlinear systems!");
}

void
FVScalarLagrangeMultiplierInterface::computeResidual(const FaceInfo & fi)
{
  setupData(fi);

  const auto r = MetaPhysicL::raw_value(_lambda[0]) * fi.faceArea() * fi.faceCoord();

  // Primal residual
  addResidualToVariable1(r);
  addResidualToVariable2(-r);

  // LM residual. We may not have any actual ScalarKernels in our simulation so we need to manually
  // make sure the scalar residuals get cached for later addition
  const auto lm_r = MetaPhysicL::raw_value(computeQpResidual()) * fi.faceArea() * fi.faceCoord();
  addResiduals(_assembly,
               std::array<Real, 1>{{lm_r}},
               _lambda_var.dofIndices(),
               _lambda_var.scalingFactor());
}

void
FVScalarLagrangeMultiplierInterface::computeJacobian(const FaceInfo & fi)
{
  setupData(fi);

  // Primal
  const auto primal_r = _lambda[0] * fi.faceArea() * fi.faceCoord();
  addResidualAndJacobianToVariable1(primal_r);
  addResidualAndJacobianToVariable2(-primal_r);

  // LM
  const auto lm_r = computeQpResidual() * fi.faceArea() * fi.faceCoord();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have one dof");
  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{lm_r}},
                          _lambda_var.dofIndices(),
                          _lambda_var.scalingFactor());
}
