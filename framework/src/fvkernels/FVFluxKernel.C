//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFluxKernel.h"

#include "MooseVariableFV.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "ADUtils.h"
#include "libmesh/elem.h"

InputParameters
FVFluxKernel::validParams()
{
  InputParameters params = FVKernel::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.registerSystemAttributeName("FVFluxKernel");
  params.addParam<bool>("force_boundary_execution",
                        false,
                        "Whether to force execution of this object on the boundary.");
  return params;
}

FVFluxKernel::FVFluxKernel(const InputParameters & params)
  : FVKernel(params),
    TwoMaterialPropertyInterface(this, blockIDs(), {}),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(
        this, false, false, /*is_fv=*/true),
    _var(*mooseVariableFV()),
    _u_elem(_var.adSln()),
    _u_neighbor(_var.adSlnNeighbor()),
    _grad_u_elem(_var.adGradSln()),
    _grad_u_neighbor(_var.adGradSlnNeighbor()),
    _force_boundary_execution(getParam<bool>("force_boundary_execution"))
{
  addMooseVariableDependency(&_var);
}

// Note the lack of quadrature point loops in the residual/jacobian compute
// functions. This is because finite volumes currently only works with
// constant monomial elements. We only have one quadrature point regardless of
// problem dimension and just multiply by the face area.

bool
FVFluxKernel::skipForBoundary(const FaceInfo & fi)
{
  if (!fi.isBoundary() || _force_boundary_execution)
    return false;

  // If we have flux bcs then we do skip
  const auto & flux_pr = _var.getFluxBCs(fi);
  if (flux_pr.first)
    return true;

  // If we don't have flux bcs *and* we do have dirichlet bcs then we don't skip. If we don't have
  // either then we assume natural boundary condition and we should skip
  return !_var.getDirichletBC(fi).first;
}

void
FVFluxKernel::computeResidual(const FaceInfo & fi)
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
    _local_re(0) = r;
    accumulateTaggedLocalResidual();
  }
  if ((ft == FaceInfo::VarFaceNeighbors::NEIGHBOR &&
       (_force_boundary_execution || _var.hasDirichletBC())) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // residual contribution of this kernel to the neighbor element
    prepareVectorTagNeighbor(_assembly, _var.number());
    _local_re(0) = -r;
    accumulateTaggedLocalResidual();
  }
}

void
FVFluxKernel::computeJacobian(Moose::DGJacobianType type, const ADReal & residual)
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

    if ((type == Moose::ElementElement || type == Moose::NeighborElement) &&
        !jvariable.activeOnSubdomain(_face_info->elemSubdomainID()))
      continue;
    else if ((type == Moose::ElementNeighbor || type == Moose::NeighborNeighbor) &&
             !jvariable.activeOnSubdomain(_face_info->neighborSubdomainID()))
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
    mooseAssert((type == Moose::ElementElement || type == Moose::NeighborElement)
                    ? jvariable.dofIndices().size() == 1
                    : jvariable.dofIndicesNeighbor().size() == 1,
                "The AD derivative indexing below only makes sense for constant monomials, e.g. "
                "for a number of dof indices equal to  1");

    _local_ke(0, 0) = residual.derivatives()[ad_offset];

    accumulateTaggedLocalMatrix();
  }
}

void
FVFluxKernel::computeJacobian(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  const ADReal r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

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

    auto element_functor = [&](const ADReal & residual, dof_id_type, const std::set<TagID> &) {
      // jacobian contribution of the residual for the elem element to the elem element's DOF:
      // d/d_elem (residual_elem)
      computeJacobian(Moose::ElementElement, residual);

      mooseAssert(
          (ft == FaceInfo::VarFaceNeighbors::ELEM) == (_var.dofIndicesNeighbor().size() == 0),
          "If the variable is only defined on the elem hand side of the face, then that "
          "means it should have no dof indices on the neighbor/neighbor element. Conversely if "
          "the variable is defined on both sides of the face, then it should have a non-zero "
          "number of degrees of freedom on the neighbor/neighbor element");

      // only add residual to neighbor if the variable is defined there.
      if (ft == FaceInfo::VarFaceNeighbors::BOTH)
        // jacobian contribution of the residual for the elem element to the neighbor element's DOF:
        // d/d_neighbor (residual_elem)
        computeJacobian(Moose::ElementNeighbor, residual);
    };

    _assembly.processDerivatives(r, _var.dofIndices()[0], _matrix_tags, element_functor);
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
    ADReal neighbor_r = -r;

    mooseAssert(_var.dofIndicesNeighbor().size() == 1,
                "We're currently built to use CONSTANT MONOMIALS");

    auto neighbor_functor = [&](const ADReal & residual, dof_id_type, const std::set<TagID> &) {
      // only add residual to elem if the variable is defined there.
      if (ft == FaceInfo::VarFaceNeighbors::BOTH)
        // jacobian contribution of the residual for the neighbor element to the elem element's DOF:
        // d/d_elem (residual_neighbor)
        computeJacobian(Moose::NeighborElement, residual);

      // jacobian contribution of the residual for the neighbor element to the neighbor element's
      // DOF: d/d_neighbor (residual_neighbor)
      computeJacobian(Moose::NeighborNeighbor, residual);
    };

    _assembly.processDerivatives(
        neighbor_r, _var.dofIndicesNeighbor()[0], _matrix_tags, neighbor_functor);
  }
}

ADReal
FVFluxKernel::gradUDotNormal() const
{
  return Moose::FV::gradUDotNormal(_u_elem[_qp], _u_neighbor[_qp], *_face_info, _var);
}
