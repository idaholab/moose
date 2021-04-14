//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVHLLCSourceBC.h"
#include "NSFVHLLCSourceKernel.h"
#include "FaceInfo.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

InputParameters
NSFVHLLCSourceBC::validParams()
{
  return CNSFVHLLCBCBase::validParams();
}

NSFVHLLCSourceBC::NSFVHLLCSourceBC(const InputParameters & parameters) : CNSFVHLLCBCBase(parameters)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("NSFVHLLCSourceKernel is only supported with global AD indexing");
#endif
}

void
NSFVHLLCSourceBC::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  // I hate the polarity switching so I'm not going to do it here like we do for FVFluxBC
  _normal = fi.normal();

  if (!NSFVHLLCSourceKernel::movingToNeighbor(_tid, fi, hllcData(), _normal))
    // This means the wind is blowing from the ghost to the elem. We don't have a source in the
    // ghost cell
    return;

  mooseAssert(fi.faceType(_var.name()) == FaceInfo::VarFaceNeighbors::ELEM,
              "Baking this in for now");
  const auto r = MetaPhysicL::raw_value(sourceElem()) * fi.faceArea() * fi.faceCoord() *
                 (fi.faceCentroid() - fi.elemCentroid()) * fi.normal();

  prepareVectorTag(_assembly, _var.number());
  _local_re(0) = r;
  accumulateTaggedLocalResidual();
}

void
NSFVHLLCSourceBC::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  // I hate the polarity switching so I'm not going to do it here like we do for FVFluxBC
  _normal = fi.normal();

  if (!NSFVHLLCSourceKernel::movingToNeighbor(_tid, fi, hllcData(), _normal))
    // This means the wind is blowing from the ghost to the elem. We don't have a source in the
    // ghost cell
    return;

  mooseAssert(fi.faceType(_var.name()) == FaceInfo::VarFaceNeighbors::ELEM,
              "Baking this in for now");
  const auto r = sourceElem() * fi.faceArea() * fi.faceCoord() *
                 (fi.faceCentroid() - fi.elemCentroid()) * fi.normal();

  _assembly.processDerivatives(r, _var.dofIndices()[0], _matrix_tags);
}

void
NSFVHLLCSourceBC::preComputeWaveSpeed()
{
  mooseError("I don't think we should ever get here.");
  _rho_boundary = _rho_elem[_qp];
  _vel_boundary = _vel_elem[_qp];
  _specific_internal_energy_boundary = _specific_internal_energy_elem[_qp];
}
