#include "FVFluxKernel.h"

#include "MooseVariableFV.h"
#include "SystemBase.h"
#include "FVDirichletBC.h"
#include "MooseMesh.h"
#include "libmesh/elem.h"

InputParameters
FVFluxKernel::validParams()
{
  InputParameters params = FVKernel::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.registerSystemAttributeName("FVFluxKernel");
  return params;
}

FVFluxKernel::FVFluxKernel(const InputParameters & params)
  : FVKernel(params),
    TwoMaterialPropertyInterface(this, blockIDs(), {}),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
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
FVFluxKernel::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto r = MetaPhysicL::raw_value(fi.faceArea() * computeQpResidual());

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
  if ((ft == FaceInfo::VarFaceNeighbors::ELEM && _var.hasDirichletBC()) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // residual contribution of this kernel to the elem element
    prepareVectorTag(_assembly, _var.number());
    _local_re(0) = r;
    accumulateTaggedLocalResidual();
  }
  if ((ft == FaceInfo::VarFaceNeighbors::RIGHT && _var.hasDirichletBC()) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // residual contribution of this kernel to the neighbor element
    prepareVectorTagNeighbor(_assembly, _var.number());
    _local_re(0) = -r;
    accumulateTaggedLocalResidual();
  }
}

void
FVFluxKernel::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  DualReal r = fi.faceArea() * computeQpResidual();

  auto & sys = _subproblem.systemBaseNonlinear();
  unsigned int dofs_per_elem = sys.getMaxVarNDofsPerElem();
  unsigned int var_num = _var.number();
  unsigned int nvars = sys.system().n_vars();

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
  if ((ft == FaceInfo::VarFaceNeighbors::ELEM && _var.hasDirichletBC()) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // jacobian contribution of the residual for the elem element to the elem element's DOF:
    // d/d_elem (residual_elem)
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::ElementElement);
    _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem];
    accumulateTaggedLocalMatrix();

    mooseAssert(
        (ft == FaceInfo::VarFaceNeighbors::ELEM) == (_var.dofIndicesNeighbor().size() == 0),
        "If the variable is only defined on the elem hand side of the face, then that "
        "means it should have no dof indices on the neighbor/neighbor element. Conversely if "
        "the variable is defined on both sides of the face, then it should have a non-zero "
        "number of degrees of freedom on the neighbor/neighbor element");

    // only add residual to neighbor if the variable is defined there.
    if (ft == FaceInfo::VarFaceNeighbors::BOTH)
    {
      // jacobian contribution of the residual for the elem element to the neighbor element's DOF:
      // d/d_neighbor (residual_elem)
      prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::ElementNeighbor);
      _local_ke(0, 0) += r.derivatives()[var_num * dofs_per_elem + nvars * dofs_per_elem];
      accumulateTaggedLocalMatrix();
    }
  }

  if ((ft == FaceInfo::VarFaceNeighbors::RIGHT && _var.hasDirichletBC()) ||
      ft == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert((ft == FaceInfo::VarFaceNeighbors::RIGHT) == (_var.dofIndices().size() == 0),
                "If the variable is only defined on the neighbor hand side of the face, then that "
                "means it should have no dof indices on the elem element. Conversely if "
                "the variable is defined on both sides of the face, then it should have a non-zero "
                "number of degrees of freedom on the elem element");

    // only add residual to elem if the variable is defined there.
    if (ft == FaceInfo::VarFaceNeighbors::BOTH)
    {
      // jacobian contribution of the residual for the neighbor element to the elem element's DOF:
      // d/d_elem (residual_neighbor)
      prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::NeighborElement);
      _local_ke(0, 0) += -1 * r.derivatives()[var_num * dofs_per_elem];
      accumulateTaggedLocalMatrix();
    }

    // jacobian contribution of the residual for the neighbor element to the neighbor element's DOF:
    // d/d_neighbor (residual_neighbor)
    prepareMatrixTagNeighbor(_assembly, var_num, var_num, Moose::NeighborNeighbor);
    _local_ke(0, 0) += -1 * r.derivatives()[var_num * dofs_per_elem + nvars * dofs_per_elem];
    accumulateTaggedLocalMatrix();
  }
}

ADReal
FVFluxKernel::gradUDotNormal()
{
  // We compute "grad_u dot _normal" by assuming the mesh is orthogonal, and
  // recognizing that it is equivalent to delta u between the two cell
  // centroids but for one unit in the normal direction.  We know delta u for
  // the length between cell centroids (u_neighbor - u_elem) and then we just
  // divide that by the distance between the centroids to convert it to delta
  // u for one unit in the normal direction.  Because the _normal vector is
  // defined to be outward from the elem element, u_neighbor-u_elem gives delta u
  // when moving in the positive normal direction.  So we divide by the
  // (positive) distance between centroids because one unit in the normal
  // direction is always positive movement.
  ADReal dudn = (_u_neighbor[_qp] - _u_elem[_qp]) /
                (_face_info->neighborCentroid() - _face_info->elemCentroid()).norm();
  // TODO: need to apply cross-diffusion correction factor here.  This
  // currently is only correct if the vector between the elem-neighbor cell
  // centroids is parallel to the normal vector.
  return dudn;
}
