//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVArrayFluxKernel.h"

#include "MooseVariableFV.h"
#include "SystemBase.h"
#include "FVDirichletBC.h"
#include "MooseMesh.h"
#include "ADUtils.h"
#include "libmesh/elem.h"

InputParameters
FVArrayFluxKernel::validParams()
{
  InputParameters params = FVFluxKernelBase::validParams();
  return params;
}

FVArrayFluxKernel::FVArrayFluxKernel(const InputParameters & params)
  : FVFluxKernelBase(params),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariableFV()),
    _u_elem(_var.adSln()),
    _u_neighbor(_var.adSlnNeighbor()),
    _grad_u_elem(_var.adGradSln()),
    _grad_u_neighbor(_var.adGradSlnNeighbor())
{
  addMooseVariableDependency(&_var);
}

// Note the lack of quadrature point loops in the residual/jacobian compute
// functions. This is because finite volumes currently only works with
// constant monomial elements. We only have one quadrature point regardless of
// problem dimension and just multiply by the face area.

void
FVArrayFluxKernel::computeResidual(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  // residual contributions for a flux kernel go to both neighboring faces.
  // They are equal in magnitude but opposite in direction due to the outward
  // facing unit normals of the face for each neighboring elements being
  // oriented oppositely.  We calculate the residual contribution once using
  // the lower-id-elem-oriented _normal and just use the resulting residual's
  // negative for the contribution to the neighbor element.

  // The fancy face type if condition checks here are because we might
  // currently be running on a face for which this kernel's variable is only
  // defined on one side. If this is the case, we need to only calculate+add
  // the residual contribution if there is a dirichlet bc for the active
  // face+variable.  We always need to add the residual contribution when the
  // variable is defined on both sides of the face.  If the variable is only
  // defined on one side and there is NOT a dirichlet BC, then there is either
  // a flux BC or a natural BC - in either of those cases we don't want to add
  // any residual contributions from regular flux kernels.
  auto ft = fi.faceType(_var.name());
  if ((ft == FaceInfo::VarFaceNeighbors::ELEM &&
       (_force_boundary_execution || _var.hasDirichletBC())) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // residual contribution of this kernel to the elem element
    prepareVectorTag(_assembly, _var.number());
    _assembly.saveLocalArrayResidual(_local_re, 0, 1, r);
    accumulateTaggedLocalResidual();
  }
  if ((ft == FaceInfo::VarFaceNeighbors::NEIGHBOR &&
       (_force_boundary_execution || _var.hasDirichletBC())) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // residual contribution of this kernel to the neighbor element
    prepareVectorTagNeighbor(_assembly, _var.number());
    _assembly.saveLocalArrayResidual(_local_re, 0, 1, -r);
    accumulateTaggedLocalResidual();
  }
}

void
FVArrayFluxKernel::computeJacobian(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  const ADRealEigenVector r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  // The fancy face type if condition checks here are because we might
  // currently be running on a face for which this kernel's variable is only
  // defined on one side. If this is the case, we need to only calculate+add
  // the jacobian contribution if there is a dirichlet bc for the active
  // face+variable.  We always need to add the jacobian contribution when the
  // variable is defined on both sides of the face.  If the variable is only
  // defined on one side and there is NOT a dirichlet BC, then there is either
  // a flux BC or a natural BC - in either of those cases we don't want to add
  // any jacobian contributions from regular flux kernels.
  auto ft = fi.faceType(_var.name());
  if ((ft == FaceInfo::VarFaceNeighbors::ELEM &&
       (_force_boundary_execution || _var.hasDirichletBC())) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

    auto element_functor = [&](const ADReal &, dof_id_type, const std::set<TagID> &) {
      mooseError("FV Array variables do not support non-global AD DOF indexing");
    };

    for (unsigned int v = 0; v < _var.count(); v++)
    {
      auto dof = _var.dofIndices()[0];
      const unsigned int ndofs = 1;
      _assembly.processDerivatives(
          r(v), Moose::globalADArrayOffset(dof, ndofs, v), _matrix_tags, element_functor);
    }
  }

  if ((ft == FaceInfo::VarFaceNeighbors::NEIGHBOR &&
       (_force_boundary_execution || _var.hasDirichletBC())) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert((ft == FaceInfo::VarFaceNeighbors::NEIGHBOR) == (_var.dofIndices().size() == 0),
                "If the variable is only defined on the neighbor hand side of the face, then that "
                "means it should have no dof indices on the elem element. Conversely if "
                "the variable is defined on both sides of the face, then it should have a non-zero "
                "number of degrees of freedom on the elem element");

    // We switch the sign for the neighbor residual
    ADRealEigenVector neighbor_r = -r;

    mooseAssert(_var.dofIndicesNeighbor().size() == 1,
                "We're currently built to use CONSTANT MONOMIALS");

    auto neighbor_functor = [&](const ADReal &, dof_id_type, const std::set<TagID> &) {
      mooseError("FV Array variables do not support non-global AD DOF indexing");
    };

    for (unsigned int v = 0; v < _var.count(); v++)
    {
      auto dof = _var.dofIndicesNeighbor()[0];
      const unsigned int ndofs = 1;
      _assembly.processDerivatives(
          neighbor_r(v), Moose::globalADArrayOffset(dof, ndofs, v), _matrix_tags, neighbor_functor);
    }
  }
}

ADRealEigenVector
FVArrayFluxKernel::gradUDotNormal() const
{
  return Moose::FV::gradUDotNormal(_u_elem[_qp], _u_neighbor[_qp], *_face_info, _var);
}
