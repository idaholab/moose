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

  // We always want the normal to point outward on boundaries so we flip it if
  // this variable is defined on the right side of the face (instead of left)
  // since the FaceInfo normal polarity is always left-elem oriented.
  if (ft == FaceInfo::VarFaceNeighbors::RIGHT)
    _normal = -_normal;

  auto r = MetaPhysicL::raw_value(fi.faceArea() * computeQpResidual());

  // This could be an "internal" boundary - one created by variable block
  // restriction where the var is only defined on one side of the face.  We
  // need to make sure that we add the residual contribution to the correct
  // side - the one where the variable is defined.
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
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  // We always want the normal to point outward on boundaries so we flip it if
  // this variable is defined on the right side of the face (instead of left)
  // since the FaceInfo normal polarity is always left-elem oriented.
  if (ft == FaceInfo::VarFaceNeighbors::RIGHT)
    _normal = -_normal;

  DualReal r = fi.faceArea() * computeQpResidual();

  auto & sys = _subproblem.systemBaseNonlinear();
  unsigned int dofs_per_elem = sys.getMaxVarNDofsPerElem();
  unsigned int var_num = _var.number();
  unsigned int nvars = sys.system().n_vars();

  // This could be an "internal" boundary - one created by variable block
  // restriction where the var is only defined on one side of the face.  We
  // need to make sure that we add the residual contribution to the correct
  // side - the one where the variable is defined.
  if (ft == FaceInfo::VarFaceNeighbors::LEFT)
  {
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::ElementElement);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem];
    accumulateTaggedLocalMatrix();

    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::ElementNeighbor);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem + nvars * dofs_per_elem];
    accumulateTaggedLocalMatrix();
  }
  else if (ft == FaceInfo::VarFaceNeighbors::RIGHT)
  {
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::NeighborElement);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem];
    accumulateTaggedLocalMatrix();

    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::NeighborNeighbor);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem + nvars * dofs_per_elem];
    accumulateTaggedLocalMatrix();
  }
  else
    mooseError("should never get here");
}

adBaseClass(FVFluxBC);
