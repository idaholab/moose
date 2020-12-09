//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVScalarLagrangeMultiplier.h"

#include "MooseVariableScalar.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

registerMooseObject("MooseApp", FVScalarLagrangeMultiplier);

InputParameters
FVScalarLagrangeMultiplier::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("This class is used to enforce integral of phi = volume * phi_0 "
                             "with a Lagrange multiplier approach.");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier variable");
  params.addParam<Real>("phi0", 0, "What we want the average value of the primal variable to be.");
  return params;
}

FVScalarLagrangeMultiplier::FVScalarLagrangeMultiplier(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _lambda_var(*getScalarVar("lambda", 0)),
    _lambda(adCoupledScalarValue("lambda")),
    _phi0(getParam<Real>("phi0"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("FVScalarLagrangeMultiplier is not supported by local AD indexing. In order to use "
             "FVScalarLagrangeMultiplier, please run the configure script in the root MOOSE "
             "directory with the configure option '--with-ad-indexing-type=global'");
#endif
}

ADReal
FVScalarLagrangeMultiplier::computeQpResidual()
{
  mooseError("There is no need for me.");
}

void
FVScalarLagrangeMultiplier::computeResidual()
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
  const auto lm_r = (MetaPhysicL::raw_value(_u[_qp]) - _phi0) * _assembly.elemVolume();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have a single dof");
  _assembly.cacheResidual(_lambda_var.dofIndices()[0], lm_r, _vector_tags);
}

void
FVScalarLagrangeMultiplier::computeJacobian()
{
}

void
FVScalarLagrangeMultiplier::computeOffDiagJacobian()
{
  // Primal
  mooseAssert(_lambda.size() == 1 && _lambda_var.order() == 1,
              "The lambda variable should be first order");
  const auto primal_r = _lambda[0] * _assembly.elemVolume();
  mooseAssert(_var.dofIndices().size() == 1, "We should only have one dof");
  _assembly.processDerivatives(primal_r, _var.dofIndices()[0], _matrix_tags);

  // LM
  const auto lm_r = (_u[_qp] - _phi0) * _assembly.elemVolume();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have one dof");
  _assembly.processDerivatives(lm_r, _lambda_var.dofIndices()[0], _matrix_tags);
}
