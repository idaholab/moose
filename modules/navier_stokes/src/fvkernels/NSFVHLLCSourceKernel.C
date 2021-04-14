//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVHLLCSourceKernel.h"
#include "FaceInfo.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

InputParameters
NSFVHLLCSourceKernel::validParams()
{
  return CNSFVHLLCBase::validParams();
}

NSFVHLLCSourceKernel::NSFVHLLCSourceKernel(const InputParameters & params) : CNSFVHLLCBase(params)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("NSFVHLLCSourceKernel is only supported with global AD indexing");
#endif
}

bool
NSFVHLLCSourceKernel::movingToNeighbor(const THREAD_ID tid,
                                       const FaceInfo & fi,
                                       const HLLCData & hllc_data,
                                       const ADRealVectorValue & normal)
{
  const auto & wave_speeds = waveSpeed(tid, fi, hllc_data, normal);
  const auto & SL = wave_speeds[0];
  const auto & SM = wave_speeds[1];
  const auto & SR = wave_speeds[2];
  bool moving_to_neighbor;
  if (SL >= 0)
    moving_to_neighbor = true;
  else if (SR <= 0)
    moving_to_neighbor = false;
  else if (SM >= 0)
    moving_to_neighbor = true;
  else
    moving_to_neighbor = false;

  return moving_to_neighbor;
}

void
NSFVHLLCSourceKernel::computeResidual(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();

  mooseAssert(fi.faceType(_var.name()) == FaceInfo::VarFaceNeighbors::BOTH,
              "With HLLC you should have FluxBCs on the boundaries");

  const auto source = fi.faceArea() * fi.faceCoord() *
                      MetaPhysicL::raw_value(movingToNeighbor(_tid, fi, hllcData(), fi.normal())
                                                 ? sourceElem()
                                                 : sourceNeighbor());

  // residual contribution of this kernel to the elem element
  prepareVectorTag(_assembly, _var.number());
  _local_re(0) = source * (fi.faceCentroid() - fi.elemCentroid()) * fi.normal();
  accumulateTaggedLocalResidual();

  // residual contribution of this kernel to the neighbor element
  prepareVectorTagNeighbor(_assembly, _var.number());
  _local_re(0) = source * (fi.neighborCentroid() - fi.faceCentroid()) * fi.normal();
  accumulateTaggedLocalResidual();
}

void
NSFVHLLCSourceKernel::computeJacobian(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();

  mooseAssert(fi.faceType(_var.name()) == FaceInfo::VarFaceNeighbors::BOTH,
              "With HLLC you should have FluxBCs on the boundaries");

  const auto source =
      fi.faceArea() * fi.faceCoord() *
      (movingToNeighbor(_tid, fi, hllcData(), fi.normal()) ? sourceElem() : sourceNeighbor());

  mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");
  _assembly.processDerivatives(source * (fi.faceCentroid() - fi.elemCentroid()) * fi.normal(),
                               _var.dofIndices()[0],
                               _matrix_tags);

  mooseAssert(_var.dofIndicesNeighbor().size() == 1,
              "We're currently built to use CONSTANT MONOMIALS");
  _assembly.processDerivatives(source * (fi.neighborCentroid() - fi.faceCentroid()) * fi.normal(),
                               _var.dofIndicesNeighbor()[0],
                               _matrix_tags);
}
