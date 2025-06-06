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

  const auto var_elem_num = _elem_is_one ? var1().number() : var2().number();
  const auto var_neigh_num = _elem_is_one ? var2().number() : var1().number();

  const auto r =
      MetaPhysicL::raw_value(_lambda[0]) * fi.faceArea() * fi.faceCoord() * (2 * _elem_is_one - 1);

  // Primal residual
  addResidual(r, var_elem_num, false);
  addResidual(-r, var_neigh_num, true);

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

  const auto & elem_dof_indices = _elem_is_one ? var1().dofIndices() : var2().dofIndices();
  const auto & neigh_dof_indices =
      _elem_is_one ? var2().dofIndicesNeighbor() : var1().dofIndicesNeighbor();
  mooseAssert((elem_dof_indices.size() == 1) && (neigh_dof_indices.size() == 1),
              "We're currently built to use CONSTANT MONOMIALS");
  const auto elem_scaling_factor = _elem_is_one ? var1().scalingFactor() : var2().scalingFactor();
  const auto neigh_scaling_factor = _elem_is_one ? var2().scalingFactor() : var1().scalingFactor();

  // Primal
  const auto primal_r = _lambda[0] * fi.faceArea() * fi.faceCoord() * (2 * _elem_is_one - 1);
  addResidualsAndJacobian(
      _assembly, std::array<ADReal, 1>{{primal_r}}, elem_dof_indices, elem_scaling_factor);
  addResidualsAndJacobian(
      _assembly, std::array<ADReal, 1>{{-primal_r}}, neigh_dof_indices, neigh_scaling_factor);

  // LM
  const auto lm_r = computeQpResidual() * fi.faceArea() * fi.faceCoord();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have one dof");
  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{lm_r}},
                          _lambda_var.dofIndices(),
                          _lambda_var.scalingFactor());
}
