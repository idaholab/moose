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
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier variable");
  return params;
}

FVScalarLagrangeMultiplierConstraint::FVScalarLagrangeMultiplierConstraint(
    const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _lambda_var(*getScalarVar("lambda", 0)),
    _lambda(adCoupledScalarValue("lambda"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError(
      "FVScalarLagrangeMultiplierConstraint is not supported by local AD indexing. In order to use "
      "FVScalarLagrangeMultiplierConstraint, please run the configure script in the root MOOSE "
      "directory with the configure option '--with-ad-indexing-type=global'");
#endif

  // If we have a point value constraint, we look for the mesh cell with the
  // constraint. For this, we use a point_locator (adapted from the DiracKernels)
  // that builds a tree to scan the mesh
  if (_constraint_type == "point-value")
  {
    if (_point_locator.get() == NULL)
    {
      _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, _mesh);
      _point_locator->enable_out_of_mesh_mode();
    }

    // We only check in the restricted blocks, if needed
    const bool block_restricted = blockIDs().find(Moose::ANY_BLOCK_ID) == blockIDs().end();
    const Elem * elem =
        block_restricted ? (*_point_locator)(_point, &blockIDs()) : (*_point_locator)(_point);

    // We communicate the results and if there is conflict between processes,
    // the minimum cell ID is chosen
    dof_id_type elem_id = elem ? elem->id() : DofObject::invalid_id;
    dof_id_type min_elem_id = elem_id;
    _mesh.comm().min(min_elem_id);

    if (min_elem_id == DofObject::invalid_id)
      mooseError("The specified point for the point-constraint Lagrange Multiplier is not in the "
                 "domain! Try alleviating block restrictions or "
                 "using another point!");

    _my_elem = min_elem_id == elem_id ? elem : NULL;
  }
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

  if (_constraint_type == "point-value")
  {
    // If _my_elem is still null on this process, we don't add anything
    if (_my_elem == _current_elem)
    {
      const auto lm_r = MetaPhysicL::raw_value(_u[_qp]) - _phi0;
      _assembly.processResidual(lm_r, _lambda_var.dofIndices()[0], _vector_tags);
    }
  }
  else
  {
    const auto lm_r = (MetaPhysicL::raw_value(_u[_qp]) - _phi0) * _assembly.elemVolume();
    _assembly.processResidual(lm_r, _lambda_var.dofIndices()[0], _vector_tags);
  }
}

void
FVScalarLagrangeMultiplierConstraint::computeJacobian()
{
  mooseError("There is no need for me.");
}

void
FVScalarLagrangeMultiplierConstraint::computeOffDiagJacobian()
{
  // Primal
  mooseAssert(_lambda.size() == 1 && _lambda_var.order() == 1,
              "The lambda variable should be first order");
  const auto primal_r = _lambda[0] * _assembly.elemVolume();
  mooseAssert(_var.dofIndices().size() == 1, "We should only have one dof");
  _assembly.processDerivatives(primal_r, _var.dofIndices()[0], _matrix_tags);

  // LM
  const auto lm_r = computeQpResidual() * _assembly.elemVolume();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have one dof");
  if (_constraint_type == "point-value")
  {
    // If _my_elem is still null on this process, we don't add anything
    if (_my_elem == _current_elem)
    {
      const auto lm_r = _u[_qp] - _phi0;
      _assembly.processDerivatives(lm_r, _lambda_var.dofIndices()[0], _matrix_tags);
    }
  }
  else
  {
    const auto lm_r = (_u[_qp] - _phi0) * _assembly.elemVolume();
    _assembly.processDerivatives(lm_r, _lambda_var.dofIndices()[0], _matrix_tags);
  }
}
