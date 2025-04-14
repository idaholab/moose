//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "RelationshipManager.h"

#include "libmesh/elem.h"
#include "libmesh/system.h"

InputParameters
FVFluxKernel::validParams()
{
  InputParameters params = FVKernel::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.registerSystemAttributeName("FVFluxKernel");
  params.addParam<bool>("force_boundary_execution",
                        false,
                        "Whether to force execution of this object on all external boundaries.");
  params.addParam<std::vector<BoundaryName>>(
      "boundaries_to_force",
      std::vector<BoundaryName>(),
      "The set of sidesets to force execution of this FVFluxKernel on. "
      "Setting force_boundary_execution to true is equivalent to listing all external "
      "mesh boundaries in this parameter.");
  params.addParam<std::vector<BoundaryName>>(
      "boundaries_to_avoid",
      std::vector<BoundaryName>(),
      "The set of sidesets to not execute this FVFluxKernel on. "
      "This takes precedence over force_boundary_execution to restrict to less external boundaries."
      " By default flux kernels are executed on all internal boundaries and Dirichlet boundary "
      "conditions.");

  params.addParamNamesToGroup("force_boundary_execution boundaries_to_force boundaries_to_avoid",
                              "Boundary execution modification");
  return params;
}

FVFluxKernel::FVFluxKernel(const InputParameters & params)
  : FVKernel(params),
    TwoMaterialPropertyInterface(this, blockIDs(), {}),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(
        this, false, false, /*is_fv=*/true),
    _var(*mooseVariableFV()),
    _u_elem(_var.adSln()),
    _u_neighbor(_var.adSlnNeighbor()),
    _force_boundary_execution(getParam<bool>("force_boundary_execution"))
{
  addMooseVariableDependency(&_var);

  const auto & vec = getParam<std::vector<BoundaryName>>("boundaries_to_force");
  for (const auto & name : vec)
    _boundaries_to_force.insert(_mesh.getBoundaryID(name));

  const auto & avoid_vec = getParam<std::vector<BoundaryName>>("boundaries_to_avoid");
  for (const auto & name : avoid_vec)
  {
    const auto bid = _mesh.getBoundaryID(name);
    _boundaries_to_avoid.insert(bid);
    if (_boundaries_to_force.find(bid) != _boundaries_to_force.end())
      paramError(
          "boundaries_to_avoid",
          "A boundary may not be specified in both boundaries_to_avoid and boundaries_to_force");
  }
}

bool
FVFluxKernel::onBoundary(const FaceInfo & fi) const
{
  return Moose::FV::onBoundary(*this, fi);
}

// Note the lack of quadrature point loops in the residual/jacobian compute
// functions. This is because finite volumes currently only works with
// constant monomial elements. We only have one quadrature point regardless of
// problem dimension and just multiply by the face area.

bool
FVFluxKernel::skipForBoundary(const FaceInfo & fi) const
{
  // Boundaries to avoid come first, since they are always obeyed
  if (avoidBoundary(fi))
    return true;

  // We get this to check if we are on a kernel boundary or not
  const bool on_boundary = onBoundary(fi);

  // We are either on a kernel boundary or on an internal sideset
  // which is handled as a boundary
  if (on_boundary || !fi.boundaryIDs().empty())
  {
    // Blanket forcing on boundary
    if (_force_boundary_execution)
      return false;

    // Selected boundaries to force
    for (const auto bnd_to_force : _boundaries_to_force)
      if (fi.boundaryIDs().count(bnd_to_force))
        return false;

    // If we have a flux boundary on this face, we skip. This
    // should be relatively easy to check with the cached maps.
    if (_var.getFluxBCs(fi).first)
      return true;

    // If we have a dirichlet BC, we are not skipping
    if (_var.getDirichletBC(fi).first)
      return false;
  }

  // The last question is: are we on the inside or on the outside? If we are on an internal
  // face we dont skip, otherwise we assume a natural BC and skip
  return on_boundary;
}

void
FVFluxKernel::computeResidual(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(std::make_pair(_var.number(), _var.sys().number()));
  auto r = fi.faceArea() * fi.faceCoord() * MetaPhysicL::raw_value(computeQpResidual());

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
  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // residual contribution of this kernel to the elem element
    prepareVectorTag(_assembly, _var.number());
    _local_re(0) = r;
    accumulateTaggedLocalResidual();
  }
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
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
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(std::make_pair(_var.number(), _var.sys().number()));
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
  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

    addResidualsAndJacobian(
        _assembly, std::array<ADReal, 1>{{r}}, _var.dofIndices(), _var.scalingFactor());
  }

  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    mooseAssert((_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR) ==
                    (_var.dofIndices().size() == 0),
                "If the variable is only defined on the neighbor hand side of the face, then that "
                "means it should have no dof indices on the elem element. Conversely if "
                "the variable is defined on both sides of the face, then it should have a non-zero "
                "number of degrees of freedom on the elem element");

    // We switch the sign for the neighbor residual
    ADReal neighbor_r = -r;

    mooseAssert(_var.dofIndicesNeighbor().size() == 1,
                "We're currently built to use CONSTANT MONOMIALS");

    addResidualsAndJacobian(_assembly,
                            std::array<ADReal, 1>{{neighbor_r}},
                            _var.dofIndicesNeighbor(),
                            _var.scalingFactor());
  }
}

void
FVFluxKernel::computeResidualAndJacobian(const FaceInfo & fi)
{
  computeJacobian(fi);
}

ADReal
FVFluxKernel::gradUDotNormal(const Moose::StateArg & time) const
{
  mooseAssert(_face_info, "the face info should be non-null");

  // Utlimately this will be a property of the kernel
  const bool correct_skewness =
      (_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage);

  return Moose::FV::gradUDotNormal(*_face_info, _var, time, correct_skewness);
}

Moose::ElemArg
FVFluxKernel::elemArg(const bool correct_skewness) const
{
  mooseAssert(_face_info, "the face info should be non-null");
  return {_face_info->elemPtr(), correct_skewness};
}

Moose::ElemArg
FVFluxKernel::neighborArg(const bool correct_skewness) const
{
  mooseAssert(_face_info, "the face info should be non-null");
  return {_face_info->neighborPtr(), correct_skewness};
}

Moose::FaceArg
FVFluxKernel::singleSidedFaceArg(const FaceInfo * fi,
                                 const Moose::FV::LimiterType limiter_type,
                                 const bool correct_skewness,
                                 const Moose::StateArg * state_limiter) const
{
  if (!fi)
    fi = _face_info;

  return makeFace(*fi, limiter_type, true, correct_skewness, state_limiter);
}

bool
FVFluxKernel::avoidBoundary(const FaceInfo & fi) const
{
  for (const auto bnd_id : fi.boundaryIDs())
    if (_boundaries_to_avoid.count(bnd_id))
      return true;
  return false;
}

void
FVFluxKernel::adjustRMGhostLayers(const unsigned short ghost_layers) const
{
  auto & factory = _app.getFactory();

  auto rm_params = factory.getValidParams("ElementSideNeighborLayers");

  rm_params.set<std::string>("for_whom") = name();
  rm_params.set<MooseMesh *>("mesh") = &const_cast<MooseMesh &>(_mesh);
  rm_params.set<Moose::RelationshipManagerType>("rm_type") =
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
      Moose::RelationshipManagerType::COUPLING;
  FVKernel::setRMParams(
      _pars, rm_params, std::max(ghost_layers, _pars.get<unsigned short>("ghost_layers")));
  mooseAssert(rm_params.areAllRequiredParamsValid(),
              "All relationship manager parameters should be valid.");

  auto rm_obj = factory.create<RelationshipManager>(
      "ElementSideNeighborLayers", name() + "_skew_correction", rm_params);

  // Delete the resources created on behalf of the RM if it ends up not being added to the
  // App.
  if (!_app.addRelationshipManager(rm_obj))
    factory.releaseSharedObjects(*rm_obj);
}

void
FVFluxKernel::computeResidual()
{
  mooseError("FVFluxKernel residual/Jacobian evaluation requires a face information object");
}

void
FVFluxKernel::computeJacobian()
{
  mooseError("FVFluxKernel residual/Jacobian evaluation requires a face information object");
}

void
FVFluxKernel::computeResidualAndJacobian()
{
  mooseError("FVFluxKernel residual/Jacobian evaluation requires a face information object");
}

bool
FVFluxKernel::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  if (fi_elem_side)
    return hasBlocks(fi.elem().subdomain_id());
  else
    return fi.neighborPtr() && hasBlocks(fi.neighbor().subdomain_id());
}
