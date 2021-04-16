//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVPressureJumpsFluxBC.h"
#include "FVUtils.h"
#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", PNSFVPressureJumpsFluxBC);

InputParameters
PNSFVPressureJumpsFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Computes eps * grad_p by approximating grad_p as a series of delta functions at faces.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<FunctionName>("p_boundary", 0, "The pressure on the boundary");
  return params;
}

PNSFVPressureJumpsFluxBC::PNSFVPressureJumpsFluxBC(const InputParameters & params)
  : FVFluxBC(params),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component")),
    _has_p_boundary(params.isParamSetByUser("p_boundary")),
    _p_boundary(getFunction("p_boundary"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError(name(), " can only be used with global AD indexing");
#endif
}

ADReal
PNSFVPressureJumpsFluxBC::computeQpResidual()
{
  return -_normal(_index) * (*_pressure_neighbor_qp - *_pressure_elem_qp);
}

void
PNSFVPressureJumpsFluxBC::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();

  auto ft = fi.faceType(_var.name());
  const ADReal ad_p_boundary = _p_boundary.value(_t, fi.faceCentroid());
  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
  {
    _pressure_elem_qp = &_pressure_elem[_qp];
    _pressure_neighbor_qp = _has_p_boundary ? &ad_p_boundary : &_pressure_elem[_qp];
  }
  else if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
  {
    _pressure_elem_qp = _has_p_boundary ? &ad_p_boundary : &_pressure_neighbor[_qp];
    _pressure_neighbor_qp = &_pressure_neighbor[_qp];
  }
  else
    mooseError("Unsupported FaceType in ", name());

  const auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
  {
    prepareVectorTag(_assembly, _var.number());
    _local_re(0) = r * _eps_elem[_qp];
    accumulateTaggedLocalResidual();
  }
  else if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
  {
    // residual contribution of this kernel to the neighbor element
    prepareVectorTagNeighbor(_assembly, _var.number());
    _local_re(0) = r * _eps_neighbor[_qp];
    accumulateTaggedLocalResidual();
  }
  else
    mooseError("Unsupported FaceType in ", name());
}

void
PNSFVPressureJumpsFluxBC::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();

  auto ft = fi.faceType(_var.name());
  const ADReal ad_p_boundary = _p_boundary.value(_t, fi.faceCentroid());
  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
  {
    _pressure_elem_qp = &_pressure_elem[_qp];
    _pressure_neighbor_qp = _has_p_boundary ? &ad_p_boundary : &_pressure_elem[_qp];
  }
  else if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
  {
    _pressure_elem_qp = _has_p_boundary ? &ad_p_boundary : &_pressure_neighbor[_qp];
    _pressure_neighbor_qp = &_pressure_neighbor[_qp];
  }
  else
    mooseError("Unsupported FaceType in ", name());

  const auto r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    _assembly.processDerivatives(r * _eps_elem[_qp], _var.dofIndices()[0], _matrix_tags);
  else if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _assembly.processDerivatives(
        r * _eps_neighbor[_qp], _var.dofIndicesNeighbor()[0], _matrix_tags);
  else
    mooseError("Unsupported FaceType in ", name());
}
