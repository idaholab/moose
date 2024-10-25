//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTurbulentAdvection.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVTurbulentAdvection);

InputParameters
INSFVTurbulentAdvection::validParams()
{
  auto params = INSFVAdvectionKernel::validParams();
  params.addClassDescription(
      "Advects an arbitrary turbulent quantity, the associated nonlinear 'variable'.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>("neglect_advection_derivatives",
                        true,
                        "Whether to remove automatic differentiation derivative terms "
                        "for velocity in the advection term");
  return params;
}

INSFVTurbulentAdvection::INSFVTurbulentAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _neglect_advection_derivatives(getParam<bool>("neglect_advection_derivatives"))
{
}

void
INSFVTurbulentAdvection::initialSetup()
{
  INSFVAdvectionKernel::initialSetup();
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
}

ADReal
INSFVTurbulentAdvection::computeQpResidual()
{
  const auto v = _rc_vel_provider.getVelocity(
      _velocity_interp_method, *_face_info, determineState(), _tid, false);
  const auto var_face = _var(makeFace(*_face_info,
                                      limiterType(_advected_interp_method),
                                      MetaPhysicL::raw_value(v) * _normal > 0),
                             determineState());
  const auto rho_face = _rho(makeFace(*_face_info,
                                      limiterType(_advected_interp_method),
                                      MetaPhysicL::raw_value(v) * _normal > 0),
                             determineState());
  if (!_neglect_advection_derivatives)
    return _normal * v * rho_face * var_face;
  else
    return _normal * MetaPhysicL::raw_value(v) * rho_face * var_face;
}

void
INSFVTurbulentAdvection::computeResidual(const FaceInfo & fi)
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
INSFVTurbulentAdvection::computeJacobian(const FaceInfo & fi)
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
