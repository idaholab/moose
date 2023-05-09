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
  params += TwoMaterialPropertyInterface::validParams();
  params.registerSystemAttributeName("FVFluxBC");

  // FVFluxBCs always rely on Boundary MaterialData
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;
  params.set<bool>("_residual_object") = true;

  return params;
}

FVFluxBC::FVFluxBC(const InputParameters & parameters)
  : FVBoundaryCondition(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(
        this, /*nodal=*/false, /*neighbor_nodal=*/false, /*is_fv=*/true),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    _u(_var.adSln()),
    _u_neighbor(_var.adSlnNeighbor())
{
  if (_var.kind() == Moose::VarKindType::VAR_AUXILIARY)
    paramError("variable",
               "There should not be a need to specify a flux "
               "boundary condition for an auxiliary variable.");
}

void
FVFluxBC::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the lower-id
  // element's perspective.  But for BCs, there is only a residual
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the neighbor side of the face (instead of elem) since
  // the FaceInfo normal polarity is always oriented with respect to the lower-id element.
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  if (_face_type == FaceInfo::VarFaceNeighbors::BOTH)
    mooseError("A FVFluxBC is being triggered on an internal face with centroid: ",
               fi.faceCentroid());
  else if (_face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    mooseError("A FVFluxBC is being triggered on a face which does not connect to a block ",
               "with the relevant finite volume variable. Its centroid: ",
               fi.faceCentroid());

  auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  // This could be an "internal" boundary - one created by variable block
  // restriction where the var is only defined on one side of the face.  We
  // need to make sure that we add the residual contribution to the correct
  // side - the one where the variable is defined.
  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM)
    prepareVectorTag(_assembly, _var.number());
  else if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    prepareVectorTagNeighbor(_assembly, _var.number());

  _local_re(0) = r;
  accumulateTaggedLocalResidual();
}

void
FVFluxBC::computeResidualAndJacobian(const FaceInfo & fi)
{
  computeJacobian(fi);
}

void
FVFluxBC::computeJacobian(const FaceInfo & fi)
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

  ADReal r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  const auto & dof_indices = (_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                                 ? _var.dofIndices()
                                 : _var.dofIndicesNeighbor();

  mooseAssert(dof_indices.size() == 1, "We're currently built to use CONSTANT MONOMIALS");

  _assembly.processResidualAndJacobian(r, dof_indices[0], _vector_tags, _matrix_tags);
}

const ADReal &
FVFluxBC::uOnUSub() const
{
  mooseAssert(_face_info, "The face info has not been set");
  const auto ft = _face_info->faceType(_var.name());
  mooseAssert(
      ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::NEIGHBOR,
      "The variable " << _var.name()
                      << " should be defined on exactly one adjacent subdomain for FVFluxBC "
                      << this->name());
  mooseAssert(_qp == 0,
              "At the time of writing, we only support one quadrature point, which should "
              "correspond to the location of the cell centroid. If that changes, we should "
              "probably change the body of FVFluxBC::uOnUSub");

  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    return _u[_qp];
  else
    return _u_neighbor[_qp];
}

const ADReal &
FVFluxBC::uOnGhost() const
{
  mooseAssert(_face_info, "The face info has not been set");
  const auto ft = _face_info->faceType(_var.name());
  mooseAssert(
      ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::NEIGHBOR,
      "The variable " << _var.name()
                      << " should be defined on exactly one adjacent subdomain for FVFluxBC "
                      << this->name());
  mooseAssert(_qp == 0,
              "At the time of writing, we only support one quadrature point, which should "
              "correspond to the location of the cell centroid. If that changes, we should "
              "probably change the body of FVFluxBC::uOnGhost");

  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    return _u_neighbor[_qp];
  else
    return _u[_qp];
}

Moose::ElemArg
FVFluxBC::elemArg(const bool correct_skewness) const
{
  return {&_face_info->elem(), correct_skewness};
}

Moose::ElemArg
FVFluxBC::neighborArg(const bool correct_skewness) const
{
  return {_face_info->neighborPtr(), correct_skewness};
}
