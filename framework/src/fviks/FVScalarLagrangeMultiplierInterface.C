//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
}

void
FVScalarLagrangeMultiplierInterface::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  _elem_is_one = sub1().find(fi.elem().subdomain_id()) != sub1().end();

#ifndef NDEBUG
  const auto ft1 = fi.faceType(var1().name());
  const auto ft2 = fi.faceType(var2().name());
  constexpr auto ft_both = FaceInfo::VarFaceNeighbors::BOTH;
  constexpr auto ft_elem = FaceInfo::VarFaceNeighbors::ELEM;
  constexpr auto ft_neigh = FaceInfo::VarFaceNeighbors::NEIGHBOR;
  mooseAssert(((ft1 == ft_both) && (ft2 == ft_both)) ||
                  (_elem_is_one && (ft1 == ft_elem) && (ft2 == ft_neigh)) ||
                  (!_elem_is_one && (ft1 == ft_neigh) && (ft2 == ft_elem)),
              "These seem like the reasonable combinations of face types.");
#endif
  mooseAssert(_lambda.size() == 1 && _lambda_var.order() == 1,
              "The lambda variable should be first order");
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have a single dof");

  const auto var_elem_num = _elem_is_one ? var1().number() : var2().number();
  const auto var_neigh_num = _elem_is_one ? var2().number() : var1().number();

  const auto r =
      MetaPhysicL::raw_value(_lambda[0]) * fi.faceArea() * fi.faceCoord() * (2 * _elem_is_one - 1);

  // Primal residual
  prepareVectorTag(_assembly, var_elem_num);
  mooseAssert(_local_re.size() == 1, "We should only have a single dof");
  _local_re(0) = r;
  accumulateTaggedLocalResidual();
  prepareVectorTagNeighbor(_assembly, var_neigh_num);
  _local_re(0) = -r;
  accumulateTaggedLocalResidual();

  // LM residual. We may not have any actual ScalarKernels in our simulation so we need to manually
  // make sure the scalar residuals get cached for later addition
  const auto lm_r = MetaPhysicL::raw_value(computeQpResidual()) * fi.faceArea() * fi.faceCoord();
  _assembly.cacheResidual(_lambda_var.dofIndices()[0], lm_r, _vector_tags);
}

void
FVScalarLagrangeMultiplierInterface::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  _elem_is_one = sub1().find(fi.elem().subdomain_id()) != sub1().end();

#ifndef NDEBUG
  const auto ft1 = fi.faceType(var1().name());
  const auto ft2 = fi.faceType(var2().name());
  constexpr auto ft_both = FaceInfo::VarFaceNeighbors::BOTH;
  constexpr auto ft_elem = FaceInfo::VarFaceNeighbors::ELEM;
  constexpr auto ft_neigh = FaceInfo::VarFaceNeighbors::NEIGHBOR;
  mooseAssert(((ft1 == ft_both) && (ft2 == ft_both)) ||
                  (_elem_is_one && (ft1 == ft_elem) && (ft2 == ft_neigh)) ||
                  (!_elem_is_one && (ft1 == ft_neigh) && (ft2 == ft_elem)),
              "These seem like the reasonable combinations of face types.");
#endif
  mooseAssert(_lambda.size() == 1 && _lambda_var.order() == 1,
              "The lambda variable should be first order");

  const auto & elem_dof_indices = _elem_is_one ? var1().dofIndices() : var2().dofIndices();
  const auto & neigh_dof_indices =
      _elem_is_one ? var2().dofIndicesNeighbor() : var1().dofIndicesNeighbor();
  mooseAssert((elem_dof_indices.size() == 1) && (neigh_dof_indices.size() == 1),
              "We're currently built to use CONSTANT MONOMIALS");

  // Primal
  const auto primal_r = _lambda[0] * fi.faceArea() * fi.faceCoord() * (2 * _elem_is_one - 1);
  _assembly.processDerivatives(primal_r, elem_dof_indices[0], _matrix_tags);
  _assembly.processDerivatives(-primal_r, neigh_dof_indices[0], _matrix_tags);

  // LM
  const auto lm_r = computeQpResidual() * fi.faceArea() * fi.faceCoord();
  mooseAssert(_lambda_var.dofIndices().size() == 1, "We should only have one dof");
  _assembly.processDerivatives(lm_r, _lambda_var.dofIndices()[0], _matrix_tags);
}
