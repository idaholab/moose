//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVFluxBC.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseVariableBase.h"

InputParameters
INSFVFluxBC::validParams()
{
  auto params = FVFluxBC::validParams();
  params += INSFVMomentumResidualObject::validParams();
  return params;
}

INSFVFluxBC::INSFVFluxBC(const InputParameters & params)
  : FVFluxBC(params), INSFVMomentumResidualObject(*this)
{
}

void
INSFVFluxBC::computeResidual(const FaceInfo & fi)
{
  if (_rc_uo.segregated())
  {
    _face_info = &fi;
    _normal = fi.normal();
    _face_type = fi.faceType(_var.name());

    if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
      _normal = -_normal;

    if (_face_type == FaceInfo::VarFaceNeighbors::BOTH)
      mooseError("A FVFluxBC is being triggered on an internal face with centroid: ",
                 fi.faceCentroid());
    else if (_face_type == FaceInfo::VarFaceNeighbors::NEITHER)
      mooseError("A FVFluxBC is being triggered on a face which does not connect to a block ",
                 "with the relevant finite volume variable. Its centroid: ",
                 fi.faceCentroid());

    auto r =
        MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeSegregatedContribution());

    if (_face_type == FaceInfo::VarFaceNeighbors::ELEM)
      prepareVectorTag(_assembly, _var.number());
    else if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
      prepareVectorTagNeighbor(_assembly, _var.number());

    _local_re(0) = r;
    accumulateTaggedLocalResidual();
  }
}

void
INSFVFluxBC::computeJacobian(const FaceInfo & fi)
{
  if (_rc_uo.segregated())
  {
    _face_info = &fi;
    _normal = fi.normal();
    _face_type = fi.faceType(_var.name());

    if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
      _normal = -_normal;

    ADReal r = fi.faceArea() * fi.faceCoord() * computeSegregatedContribution();

    const auto & dof_indices = (_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                                   ? _var.dofIndices()
                                   : _var.dofIndicesNeighbor();

    mooseAssert(dof_indices.size() == 1, "We're currently built to use CONSTANT MONOMIALS");

    addResidualsAndJacobian(
        _assembly, std::array<ADReal, 1>{{r}}, dof_indices, _var.scalingFactor());
  }
}

void
INSFVFluxBC::computeResidualAndJacobian(const FaceInfo & fi)
{
  if (_rc_uo.segregated())
    computeJacobian(fi);
}

void
INSFVFluxBC::addResidualAndJacobian(const ADReal & residual)
{
  const auto * const elem = (_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                                ? &_face_info->elem()
                                : _face_info->neighborPtr();
  const auto dof_index = elem->dof_number(_sys.number(), _var.number(), 0);

  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{residual}},
                          std::array<dof_id_type, 1>{{dof_index}},
                          _var.scalingFactor());
}
