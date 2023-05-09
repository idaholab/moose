//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVBoundaryScalarLagrangeMultiplierConstraint.h"

#include "MooseVariableScalar.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

InputParameters
FVBoundaryScalarLagrangeMultiplierConstraint::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Base class for imposing constraints using scalar Lagrange multipliers");
  params.addParam<PostprocessorName>("phi0", "0", "The value that the constraint will enforce.");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier variable");
  return params;
}

FVBoundaryScalarLagrangeMultiplierConstraint::FVBoundaryScalarLagrangeMultiplierConstraint(
    const InputParameters & parameters)
  : FVFluxBC(parameters),
    _phi0(getPostprocessorValue("phi0")),
    _lambda_var(*getScalarVar("lambda", 0)),
    _lambda(adCoupledScalarValue("lambda"))
{
}

void
FVBoundaryScalarLagrangeMultiplierConstraint::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the lower-id
  // element's perspective.  But for BCs, there is only a Jacobian
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the neighbor side of the face (instead of elem) since
  // the FaceInfo normal polarity is always oriented with respect to the lower-id element.
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  // Primal
  prepareVectorTag(_assembly, _var.number());
  mooseAssert(_local_re.size() == 1, "We should only have a single dof");
  mooseAssert(_lambda.size() == 1 && _lambda_var.order() == 1,
              "The lambda variable should be first order");
  _local_re(0) += MetaPhysicL::raw_value(_lambda[0]) * fi.faceArea() * fi.faceCoord();
  accumulateTaggedLocalResidual();

  // LM
  const auto lm_r = MetaPhysicL::raw_value(computeQpResidual()) * fi.faceArea() * fi.faceCoord();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have a single dof");
  addResiduals(_assembly,
               std::array<Real, 1>{{lm_r}},
               _lambda_var.dofIndices(),
               _lambda_var.scalingFactor());
}

void
FVBoundaryScalarLagrangeMultiplierConstraint::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the lower-id
  // element's perspective.  But for BCs, there is only a Jacobian
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the neighbor side of the face (instead of elem) since
  // the FaceInfo normal polarity is always oriented with respect to the lower-id element.
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  const auto & dof_indices = (_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                                 ? _var.dofIndices()
                                 : _var.dofIndicesNeighbor();

  mooseAssert(dof_indices.size() == 1, "We're currently built to use CONSTANT MONOMIALS");

  // Primal
  mooseAssert(_lambda.size() == 1 && _lambda_var.order() == 1,
              "The lambda variable should be first order");
  const auto primal_r = _lambda[0] * (fi.faceArea() * fi.faceCoord());
  addResidualsAndJacobian(
      _assembly, std::array<ADReal, 1>{{primal_r}}, dof_indices, _var.scalingFactor());

  // LM
  const auto lm_r = computeQpResidual() * (fi.faceArea() * fi.faceCoord());
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have one dof");
  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{lm_r}},
                          _lambda_var.dofIndices(),
                          _lambda_var.scalingFactor());
}
