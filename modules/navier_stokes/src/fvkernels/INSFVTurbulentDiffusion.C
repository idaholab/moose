//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTurbulentDiffusion.h"

registerMooseObject("MooseApp", INSFVTurbulentDiffusion);

InputParameters
INSFVTurbulentDiffusion::validParams()
{
  InputParameters params = FVDiffusion::validParams();
  params.addClassDescription(
      "Computes residual for the turbulent scaled diffusion operator for finite volume method.");
  params.addParam<MooseFunctorName>("scaling_coef", 1.0, "diffusion coefficient");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<std::vector<BoundaryName>>("walls", "Boundaries that correspond to solid walls.");

  return params;
}

INSFVTurbulentDiffusion::INSFVTurbulentDiffusion(const InputParameters & params)
  : FVDiffusion(params),
    _scaling_coef(getFunctor<ADReal>("scaling_coef")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
  for (const auto & elem : _fe_problem.mesh().getMesh().element_ptr_range())
  {
    auto wall_bounded = false;
    for (unsigned int i_side = 0; i_side < elem->n_sides(); ++i_side)
    {
      const std::vector<BoundaryID> side_bnds = _subproblem.mesh().getBoundaryIDs(elem, i_side);
      for (const BoundaryName & name : _wall_boundary_names)
      {
        BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
        for (BoundaryID side_id : side_bnds)
        {
          if (side_id == wall_id)
            wall_bounded = true;
        }
      }
    }
    _wall_bounded[elem] = wall_bounded;
  }
}

ADReal
INSFVTurbulentDiffusion::computeQpResidual()
{
  using namespace Moose::FV;

  auto dudn = gradUDotNormal();
  ADReal coeff;
  ADReal scaling_coef;

  // If we are on internal faces, we interpolate the diffusivity as usual
  if (_var.isInternalFace(*_face_info))
  {
    interpolate(
        _coeff_interp_method, coeff, _coeff(elemArg()), _coeff(neighborArg()), *_face_info, true);
    interpolate(_coeff_interp_method,
                scaling_coef,
                _scaling_coef(elemArg()),
                _scaling_coef(neighborArg()),
                *_face_info,
                true);
  }
  // Else we just use the boundary values (which depend on how the diffusion
  // coefficient is constructed)
  else
  {
    const auto face = singleSidedFaceArg();
    coeff = _coeff(face);
    scaling_coef = _scaling_coef(face);
  }

  return -1 * coeff / scaling_coef * dudn;
}

void
INSFVTurbulentDiffusion::computeResidual(const FaceInfo & fi)
{

  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());
  auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  const Elem * elem = fi.elemPtr();
  const Elem * neighor = fi.neighborPtr();
  auto bounded_elem = _wall_bounded[elem];
  auto bounded_neigh = _wall_bounded[neighor];

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    if (!bounded_elem)
    {
      // residual contribution of this kernel to the elem element
      prepareVectorTag(_assembly, _var.number());
      _local_re(0) = r;
      accumulateTaggedLocalResidual();
    }
  }
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    if (!bounded_neigh)
    {
      // residual contribution of this kernel to the neighbor element
      prepareVectorTagNeighbor(_assembly, _var.number());
      _local_re(0) = -r;
      accumulateTaggedLocalResidual();
    }
  }
}

void
INSFVTurbulentDiffusion::computeJacobian(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());
  const ADReal r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  const Elem * elem = fi.elemPtr();
  const Elem * neighbor = fi.neighborPtr();
  auto bounded_elem = _wall_bounded[elem];
  auto bounded_neigh = _wall_bounded[neighbor];

  if ((_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
       _face_type == FaceInfo::VarFaceNeighbors::BOTH) &&
      (!bounded_elem))
  {
    mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

#ifdef MOOSE_GLOBAL_AD_INDEXING
    _assembly.processResidualAndJacobian(r, _var.dofIndices()[0], _vector_tags, _matrix_tags);
#else
    auto element_functor = [&](const ADReal & residual, dof_id_type, const std::set<TagID> &)
    {
      // jacobian contribution of the residual for the elem element to the elem element's DOF:
      // d/d_elem (residual_elem)
      computeJacobianType(Moose::ElementElement, residual);

      mooseAssert(
          (_face_type == FaceInfo::VarFaceNeighbors::ELEM) ==
              (_var.dofIndicesNeighbor().size() == 0),
          "If the variable is only defined on the elem hand side of the face, then that "
          "means it should have no dof indices on the neighbor/neighbor element. Conversely if "
          "the variable is defined on both sides of the face, then it should have a non-zero "
          "number of degrees of freedom on the neighbor/neighbor element");

      // only add residual to neighbor if the variable is defined there.
      if (_face_type == FaceInfo::VarFaceNeighbors::BOTH)
        // jacobian contribution of the residual for the elem element to the neighbor element's DOF:
        // d/d_neighbor (residual_elem)
        computeJacobianType(Moose::ElementNeighbor, residual);
    };
    _assembly.processJacobian(r, _var.dofIndices()[0], _matrix_tags, element_functor);
#endif
  }

  if ((_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
       _face_type == FaceInfo::VarFaceNeighbors::BOTH) &&
      (!bounded_neigh))
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

#ifdef MOOSE_GLOBAL_AD_INDEXING
    _assembly.processResidualAndJacobian(
        neighbor_r, _var.dofIndicesNeighbor()[0], _vector_tags, _matrix_tags);
#else
    auto neighbor_functor = [&](const ADReal & residual, dof_id_type, const std::set<TagID> &)
    {
      // only add residual to elem if the variable is defined there.
      if (_face_type == FaceInfo::VarFaceNeighbors::BOTH)
        // jacobian contribution of the residual for the neighbor element to the elem element's DOF:
        // d/d_elem (residual_neighbor)
        computeJacobianType(Moose::NeighborElement, residual);

      // jacobian contribution of the residual for the neighbor element to the neighbor element's
      // DOF: d/d_neighbor (residual_neighbor)
      computeJacobianType(Moose::NeighborNeighbor, residual);
    };

    _assembly.processJacobian(
        neighbor_r, _var.dofIndicesNeighbor()[0], _matrix_tags, neighbor_functor);
#endif
  }
}
