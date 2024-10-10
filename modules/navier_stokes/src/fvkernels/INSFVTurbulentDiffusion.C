//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTurbulentDiffusion.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", INSFVTurbulentDiffusion);

InputParameters
INSFVTurbulentDiffusion::validParams()
{
  InputParameters params = FVDiffusion::validParams();
  params.addClassDescription(
      "Computes residual for the turbulent scaled diffusion operator for finite volume method.");
  params.addParam<MooseFunctorName>(
      "scaling_coef", 1.0, "Scaling factor to divide the diffusion coefficient with");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");

  return params;
}

INSFVTurbulentDiffusion::INSFVTurbulentDiffusion(const InputParameters & params)
  : FVDiffusion(params),
    _scaling_coef(getFunctor<ADReal>("scaling_coef")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _preserve_sparsity_pattern(_fe_problem.preserveMatrixSparsityPattern())
{
}

void
INSFVTurbulentDiffusion::initialSetup()
{
  FVDiffusion::initialSetup();
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
}

ADReal
INSFVTurbulentDiffusion::computeQpResidual()
{
  using namespace Moose::FV;
  const auto state = determineState();

  const auto dudn = gradUDotNormal(state);
  ADReal coeff;
  ADReal scaling_coef;

  // If we are on internal faces, we interpolate the diffusivity as usual
  if (_var.isInternalFace(*_face_info))
  {
    // If the diffusion coefficients are zero, then we can early return 0 (and avoid warnings if we
    // have a harmonic interpolation)
    const auto coeff_elem = _coeff(elemArg(), state);
    const auto coeff_neighbor = _coeff(neighborArg(), state);
    if (!coeff_elem.value() && !coeff_neighbor.value())
    {
      if (!_preserve_sparsity_pattern)
        return 0;
      else
        // we return 0 but preserve the sparsity pattern of the Jacobian for Newton's method
        return 0 * (coeff_elem + coeff_neighbor) *
               (_scaling_coef(elemArg(), state) + _scaling_coef(neighborArg(), state)) * dudn;
    }

    interpolate(_coeff_interp_method, coeff, coeff_elem, coeff_neighbor, *_face_info, true);
    interpolate(_coeff_interp_method,
                scaling_coef,
                _scaling_coef(elemArg(), state),
                _scaling_coef(neighborArg(), state),
                *_face_info,
                true);
  }
  // Else we just use the boundary values (which depend on how the diffusion
  // coefficient is constructed)
  else
  {
    const auto face = singleSidedFaceArg();
    coeff = _coeff(face, state);
    scaling_coef = _scaling_coef(face, state);
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
  _face_type = _face_info->faceType(std::make_pair(_var.number(), _var.sys().number()));
  auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  const Elem * elem = fi.elemPtr();
  const Elem * neighbor = fi.neighborPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();
  const auto bounded_neigh = _wall_bounded.find(neighbor) != _wall_bounded.end();

  if ((_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
       _face_type == FaceInfo::VarFaceNeighbors::BOTH) &&
      (!bounded_elem))
  {
    // residual contribution of this kernel to the elem element
    prepareVectorTag(_assembly, _var.number());
    _local_re(0) = r;
    accumulateTaggedLocalResidual();
  }
  if ((_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
       _face_type == FaceInfo::VarFaceNeighbors::BOTH) &&
      (!bounded_neigh))
  {
    // residual contribution of this kernel to the neighbor element
    prepareVectorTagNeighbor(_assembly, _var.number());
    _local_re(0) = -r;
    accumulateTaggedLocalResidual();
  }
}

void
INSFVTurbulentDiffusion::computeJacobian(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = _face_info->faceType(std::make_pair(_var.number(), _var.sys().number()));
  const ADReal r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  const Elem * elem = fi.elemPtr();
  const Elem * neighbor = fi.neighborPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();
  const auto bounded_neigh = _wall_bounded.find(neighbor) != _wall_bounded.end();

  if ((_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
       _face_type == FaceInfo::VarFaceNeighbors::BOTH) &&
      (!bounded_elem))
  {
    mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

    addResidualsAndJacobian(
        _assembly, std::array<ADReal, 1>{{r}}, _var.dofIndices(), _var.scalingFactor());
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

    addResidualsAndJacobian(_assembly,
                            std::array<ADReal, 1>{{neighbor_r}},
                            _var.dofIndicesNeighbor(),
                            _var.scalingFactor());
  }
}
