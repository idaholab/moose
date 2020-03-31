//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFluxBC.h"
#include "MooseVariableFV.h"

InputParameters
FVFluxBCBase::validParams()
{
  InputParameters params = FVBoundaryCondition::validParams();
  return params;
}

FVFluxBCBase::FVFluxBCBase(const InputParameters & params)
  : FVBoundaryCondition(params)
{
}

template <ComputeStage compute_stage>
InputParameters
FVFluxBC<compute_stage>::validParams()
{
  InputParameters params = FVFluxBCBase::validParams();
  return params;
}

template <ComputeStage compute_stage>
FVFluxBC<compute_stage>::FVFluxBC(const InputParameters & parameters)
  : FVFluxBCBase(parameters), _u(_var.adSln<compute_stage>())
{
}

template <ComputeStage compute_stage>
void
FVFluxBC<compute_stage>::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  // We always want the normal to point outward on boundaries
  // so we flip it if this variabele is defined on the right side
  // of the face
  if (ft == FaceInfo::VarFaceNeighbors::RIGHT)
    _normal = -_normal;

  // compute the residual contribution and immediately multiply
  // by face area
  auto r = MetaPhysicL::raw_value(fi.faceArea() * computeQpResidual());

  if (ft == FaceInfo::VarFaceNeighbors::LEFT)
    prepareVectorTag(_assembly, _var.number());
  else if (ft == FaceInfo::VarFaceNeighbors::RIGHT)
    prepareVectorTagNeighbor(_assembly, _var.number());
  else
    mooseError("should never get here");

  _local_re(0) = r;
  accumulateTaggedLocalResidual();
}

template <>
void
FVFluxBC<RESIDUAL>::computeJacobian(const FaceInfo & /*fi*/)
{
}

template <ComputeStage compute_stage>
void
FVFluxBC<compute_stage>::computeJacobian(const FaceInfo & fi)
{
  // Jacobians are not evaluated right now.
}

adBaseClass(FVFluxBC);
