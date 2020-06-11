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
#include "SystemBase.h"
#include "Assembly.h"
#include "ADUtils.h"

InputParameters
FVFluxBC::validParams()
{
  InputParameters params = FVBoundaryCondition::validParams();
  params += MaterialPropertyInterface::validParams();
  params.registerSystemAttributeName("FVFluxBC");
  return params;
}

FVFluxBC::FVFluxBC(const InputParameters & parameters)
  : FVBoundaryCondition(parameters),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    _u(_var.adSln())
{
}

void
FVFluxBC::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the lower-id
  // element's perspective.  But for BCs, there is only a residual
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the neighbor side of the face (instead of elem) since
  // the FaceInfo normal polarity is always oriented with respect to the lower-id element.
  if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  // This could be an "internal" boundary - one created by variable block
  // restriction where the var is only defined on one side of the face.  We
  // need to make sure that we add the residual contribution to the correct
  // side - the one where the variable is defined.
  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    prepareVectorTag(_assembly, _var.number());
  else if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    prepareVectorTagNeighbor(_assembly, _var.number());
  else
    mooseError("should never get here");

  _local_re(0) = r;
  accumulateTaggedLocalResidual();
}

void
FVFluxBC::computeJacobian(Moose::DGJacobianType type, const ADReal & residual)
{
  auto & ce = _assembly.couplingEntries();
  for (const auto & it : ce)
  {
    MooseVariableFieldBase & ivariable = *(it.first);
    MooseVariableFieldBase & jvariable = *(it.second);

    // We currently only support coupling to other FV variables
    // Remove this when we enable support for it.
    if (!jvariable.isFV())
      continue;

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivar != _var.number())
      continue;

    SystemBase & sys = _subproblem.systemBaseNonlinear();
    auto dofs_per_elem = sys.getMaxVarNDofsPerElem();

    auto ad_offset = Moose::adOffset(jvar, dofs_per_elem, type, sys.system().n_vars());

    prepareMatrixTagNeighbor(_assembly, ivar, jvar, type);

    mooseAssert(
        _local_ke.m() == 1,
        "We are currently only supporting constant monomials for finite volume calculations");
    mooseAssert(
        _local_ke.n() == 1,
        "We are currently only supporting constant monomials for finite volume calculations");
    mooseAssert(jvariable.dofIndices().size() == 1,
                "The AD derivative indexing below only makes sense for constant monomials, e.g. "
                "for a number of dof indices equal to  1");

    _local_ke(0, 0) = residual.derivatives()[ad_offset];

    accumulateTaggedLocalMatrix();
  }
}

void
FVFluxBC::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the lower-id
  // element's perspective.  But for BCs, there is only a Jacobian
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the neighbor side of the face (instead of elem) since
  // the FaceInfo normal polarity is always oriented with respect to the lower-id element.
  if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  DualReal r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  // Even though the elem element is always the non-null pointer on mesh
  // external boundary faces, this could be an "internal" boundary - one
  // created by variable block restriction where the var is only defined on
  // one side of the face (either elem or neighbor).  We need to make sure
  // that we add the residual contribution to only the correct side - the one
  // where the variable is defined.
  // Also, we don't need to worry about ElementNeighbor or NeighborElement
  // contributions here because, once again, this is a boundary face with the
  // variable only defined on one side.
  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    computeJacobian(Moose::ElementElement, r);
  else if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    computeJacobian(Moose::NeighborNeighbor, r);
  else
    mooseError("should never get here");
}
