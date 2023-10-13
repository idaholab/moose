//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVSymmetryVelocityBC.h"

registerMooseObject("NavierStokesApp", INSFVSymmetryVelocityBC);

InputParameters
INSFVSymmetryVelocityBC::validParams()
{
  InputParameters params = INSFVFluxBC::validParams();
  params += INSFVSymmetryBC::validParams();
  params.addClassDescription("Implements a symmetry boundary condition for the velocity.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", 0, "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", 0, "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("mu", "The viscosity");
  return params;
}

INSFVSymmetryVelocityBC::INSFVSymmetryVelocityBC(const InputParameters & params)
  : INSFVFluxBC(params),
    INSFVSymmetryBC(params),
    _u_functor(getFunctor<ADReal>("u")),
    _v_functor(getFunctor<ADReal>("v")),
    _w_functor(getFunctor<ADReal>("w")),
    _mu(getFunctor<ADReal>("mu")),
    _dim(_subproblem.mesh().dimension())
{
}

ADReal
INSFVSymmetryVelocityBC::computeSegregatedContribution()
{
  const bool use_elem = _face_info->faceType(_var.name()) == FaceInfo::VarFaceNeighbors::ELEM;
  const auto elem_arg =
      use_elem ? makeElemArg(&_face_info->elem()) : makeElemArg(_face_info->neighborPtr());

  const auto state = determineState();
  const auto state_old = Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);

  const auto normal = use_elem ? _face_info->normal() : Point(-_face_info->normal());
  const auto & cell_centroid =
      use_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();

  ADRealVectorValue vel_C;
  vel_C(0) = _u_functor(elem_arg, state);
  if (_dim > 1)
  {
    vel_C(1) = _v_functor(elem_arg, state);
    if (_dim > 2)
      vel_C(2) = _w_functor(elem_arg, state);
  }

  ADRealVectorValue vel_C_old;
  vel_C_old(0) = _u_functor(elem_arg, state_old);
  if (_dim > 1)
  {
    vel_C_old(1) = _v_functor(elem_arg, state_old);
    if (_dim > 2)
      vel_C_old(2) = _w_functor(elem_arg, state_old);
  }

  const auto d_perpendicular = std::abs((_face_info->faceCentroid() - cell_centroid) * normal);

  const auto face = singleSidedFaceArg();
  const auto mu_b = _mu(face, state);

  auto normal_x_normal = outer_product(normal, normal);
  for (const auto dim_i : make_range(_dim))
    normal_x_normal(dim_i, dim_i) = 0.0;

  const auto normal_squared = normal(_index) * normal(_index);

  const auto face_flux =
      _rc_uo.getVelocity(Moose::FV::InterpMethod::RhieChow, *_face_info, state, _tid) * normal;

  ADReal matrix_contribution;
  ADReal rhs_contribution;

  // First, add the advective flux
  matrix_contribution = face_flux * (1 - normal_squared) * vel_C(_index);
  rhs_contribution = face_flux * (normal_x_normal * vel_C_old)(_index);

  // Second, add the diffusive flux
  matrix_contribution += mu_b / d_perpendicular * normal_squared * vel_C(_index);
  rhs_contribution += mu_b / d_perpendicular * (normal_x_normal * vel_C_old)(_index);

  return matrix_contribution - rhs_contribution;
}

void
INSFVSymmetryVelocityBC::gatherRCData(const FaceInfo & fi)
{
  _face_info = &fi;
  _face_type = fi.faceType(_var.name());

  const bool use_elem = _face_info->faceType(_var.name()) == FaceInfo::VarFaceNeighbors::ELEM;
  const auto elem_arg =
      use_elem ? makeElemArg(&_face_info->elem()) : makeElemArg(_face_info->neighborPtr());
  const auto state = determineState();
  const auto normal = use_elem ? _face_info->normal() : Point(-_face_info->normal());
  const Point & cell_centroid =
      use_elem ? _face_info->elemCentroid() : _face_info->neighborCentroid();
  const auto u_C = _u_functor(elem_arg, state);
  const auto v_C = _v_functor(elem_arg, state);
  const auto w_C = _w_functor(elem_arg, state);

  const auto d_perpendicular = std::abs((_face_info->faceCentroid() - cell_centroid) * normal);

  // See Moukalled 15.150. Recall that we multiply by the area in the base class, so S_b ->
  // normal.norm() -> 1 here.

  const auto face = singleSidedFaceArg();
  const auto mu_b = _mu(face, state);

  ADReal v_dot_n = u_C * normal(0);
  if (_dim > 1)
    v_dot_n += v_C * normal(1);
  if (_dim > 2)
    v_dot_n += w_C * normal(2);

  const auto strong_resid = mu_b / d_perpendicular * normal(_index) * v_dot_n;

  // The strong residual for this object is a superposition of all the velocity components, however,
  // for the on-diagonal 'a' coefficient, we only care about the coefficient multiplying the
  // velocity component corresponding to _index, hence v_dot_n -> normal(_index) when moving from
  // strong_resid -> a
  const auto a = mu_b / d_perpendicular * normal(_index) * normal(_index);

  _rc_uo.addToA((_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? &fi.elem() : fi.neighborPtr(),
                _index,
                a * (fi.faceArea() * fi.faceCoord()));

  addResidualAndJacobian(strong_resid * (fi.faceArea() * fi.faceCoord()));
}
