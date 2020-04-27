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

InputParameters
FVFluxBC::validParams()
{
  InputParameters params = FVBoundaryCondition::validParams();
  params.registerSystemAttributeName("FVFluxBC");
  return params;
}

FVFluxBC::FVFluxBC(const InputParameters & parameters)
  : FVBoundaryCondition(parameters), _u(_var.adSln())
{
}

void
FVFluxBC::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the elem
  // element's perspective.  But for BCs, there is only a residual
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the right side of the face (instead of elem) since
  // the FaceInfo normal polarity is always elem-elem oriented.
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

void
FVFluxBC::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the elem
  // element's perspective.  But for BCs, there is only a residual
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the right side of the face (instead of elem) since
  // the FaceInfo normal polarity is always elem-elem oriented.
  if (ft == FaceInfo::VarFaceNeighbors::RIGHT)
    _normal = -_normal;

  DualReal r = fi.faceArea() * computeQpResidual();

  auto & sys = _subproblem.systemBaseNonlinear();
  unsigned int dofs_per_elem = sys.getMaxVarNDofsPerElem();
  unsigned int var_num = _var.number();
  unsigned int nvars = sys.system().n_vars();

  // This could be an "internal" boundary - one created by variable block
  // restriction where the var is only defined on one side of the face.  We
  // need to make sure that we add the residual contribution to only the correct
  // side - the one where the variable is defined.
  if (ft == FaceInfo::VarFaceNeighbors::LEFT)
  {
    // jacobian contribution of the residual for the elem element to the elem element's DOF:
    // d/d_elem (residual_elem)
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::ElementElement);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem];
    accumulateTaggedLocalMatrix();

    // jacobian contribution of the residual for the elem element to the right element's DOF:
    // d/d_right (residual_elem)
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::ElementNeighbor);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem + nvars * dofs_per_elem];
    accumulateTaggedLocalMatrix();
  }
  else if (ft == FaceInfo::VarFaceNeighbors::RIGHT)
  {
    // jacobian contribution of the residual for the right element to the elem element's DOF:
    // d/d_elem (residual_right)
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::NeighborElement);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem];
    accumulateTaggedLocalMatrix();

    // jacobian contribution of the residual for the right element to the right element's DOF:
    // d/d_right (residual_right)
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::NeighborNeighbor);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem + nvars * dofs_per_elem];
    accumulateTaggedLocalMatrix();
  }
  else
    mooseError("should never get here");
}
