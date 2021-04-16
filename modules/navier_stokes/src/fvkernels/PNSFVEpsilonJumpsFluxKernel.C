//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVEpsilonJumpsFluxKernel.h"
#include "FVUtils.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PNSFVEpsilonJumpsFluxKernel);

InputParameters
PNSFVEpsilonJumpsFluxKernel::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes p * grad_eps when grad_eps is a delta function at a face.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

PNSFVEpsilonJumpsFluxKernel::PNSFVEpsilonJumpsFluxKernel(const InputParameters & params)
  : FVFluxKernel(params),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError(name(), " can only be used with global AD indexing");
#endif
}

ADReal
PNSFVEpsilonJumpsFluxKernel::computeQpResidual()
{
  return -_normal(_index) * (_eps_neighbor[_qp] - _eps_elem[_qp]) / 2;
}

void
PNSFVEpsilonJumpsFluxKernel::computeResidual(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();

  const auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  auto ft = fi.faceType(_var.name());
  if (ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // residual contribution of this kernel to the elem element
    prepareVectorTag(_assembly, _var.number());
    _local_re(0) = r * MetaPhysicL::raw_value(_pressure_elem[_qp]);
    accumulateTaggedLocalResidual();
  }
  if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR || ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // residual contribution of this kernel to the neighbor element
    prepareVectorTagNeighbor(_assembly, _var.number());
    _local_re(0) = r * MetaPhysicL::raw_value(_pressure_neighbor[_qp]);
    accumulateTaggedLocalResidual();
  }
}

void
PNSFVEpsilonJumpsFluxKernel::computeJacobian(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  const auto r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  auto ft = fi.faceType(_var.name());
  if (ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");
    _assembly.processDerivatives(r * _pressure_elem[_qp], _var.dofIndices()[0], _matrix_tags);
  }

  if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR || ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert((ft == FaceInfo::VarFaceNeighbors::NEIGHBOR) == (_var.dofIndices().size() == 0),
                "If the variable is only defined on the neighbor hand side of the face, then that "
                "means it should have no dof indices on the elem element. Conversely if "
                "the variable is defined on both sides of the face, then it should have a non-zero "
                "number of degrees of freedom on the elem element");

    mooseAssert(_var.dofIndicesNeighbor().size() == 1,
                "We're currently built to use CONSTANT MONOMIALS");
    _assembly.processDerivatives(
        r * _pressure_neighbor[_qp], _var.dofIndicesNeighbor()[0], _matrix_tags);
  }
}
