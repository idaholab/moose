//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BernoulliPressureVariable.h"
#include "NS.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", BernoulliPressureVariable);

InputParameters
BernoulliPressureVariable::validParams()
{
  auto params = INSFVPressureVariable::validParams();
  params += ADFunctorInterface::validParams();
  params.addRequiredParam<MooseFunctorName>("u", "The x-component of velocity");
  params.addParam<MooseFunctorName>("v", 0, "The y-component of velocity");
  params.addParam<MooseFunctorName>("w", 0, "The z-component of velocity");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "The porosity");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density");
  params.addParam<bool>(
      "allow_two_term_expansion_on_bernoulli_faces",
      false,
      "Switch to enable the two-term extrapolation on porosity jump faces. "
      "WARNING: This might lead to crushes in parallel runs if porosity jump faces are connected "
      "with one cell (usually corners) due to the insufficient number of ghosted "
      "layers.");
  return params;
}

BernoulliPressureVariable::BernoulliPressureVariable(const InputParameters & params)
  : INSFVPressureVariable(params),
    ADFunctorInterface(this),
    _u(nullptr),
    _v(nullptr),
    _w(nullptr),
    _eps(nullptr),
    _rho(nullptr),
    _allow_two_term_expansion_on_bernoulli_faces(
        getParam<bool>("allow_two_term_expansion_on_bernoulli_faces"))
{
  if (_allow_two_term_expansion_on_bernoulli_faces &&
      _face_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage)
    paramError(
        "allow_two_term_expansion_on_bernoulli_faces",
        "Skewness correction with two term extrapolation on Bernoulli faces is not supported!");
}

void
BernoulliPressureVariable::initialSetup()
{
  _u = &getFunctor<ADReal>("u", _subproblem);
  _v = &getFunctor<ADReal>("v", _subproblem);
  _w = &getFunctor<ADReal>("w", _subproblem);
  _eps = &getFunctor<ADReal>(NS::porosity, _subproblem);
  _rho = &getFunctor<ADReal>(NS::density, _subproblem);

  INSFVPressureVariable::initialSetup();
}

std::pair<bool, ADRealVectorValue>
BernoulliPressureVariable::elemIsUpwind(const Elem & elem,
                                        const FaceInfo & fi,
                                        const Moose::StateArg & time) const
{
  const Moose::FaceArg face{&fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr};

  const VectorValue<ADReal> vel_face{(*_u)(face, time), (*_v)(face, time), (*_w)(face, time)};
  const bool fi_elem_is_upwind = vel_face * fi.normal() > 0;
  const bool elem_is_upwind = &elem == &fi.elem() ? fi_elem_is_upwind : !fi_elem_is_upwind;

  return {elem_is_upwind, vel_face};
}

bool
BernoulliPressureVariable::isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                                      const Elem * const elem,
                                                      const Moose::StateArg & time) const
{
  if (isDirichletBoundaryFace(fi, elem, time))
    return false;
  if (!isInternalFace(fi))
    // We are neither a Dirichlet nor an internal face
    return true;

  // If we got here, then we're definitely on an internal face

  if (std::get<0>(NS::isPorosityJumpFace(*_eps, fi, time)))
    // We choose to extrapolate for the element that is downwind
    return !std::get<0>(elemIsUpwind(*elem, fi, time));

  return false;
}

bool
BernoulliPressureVariable::isDirichletBoundaryFace(const FaceInfo & fi,
                                                   const Elem * const elem,
                                                   const Moose::StateArg & time) const
{
  if (INSFVPressureVariable::isDirichletBoundaryFace(fi, elem, time))
    return true;

  if (isInternalFace(fi) && std::get<0>(NS::isPorosityJumpFace(*_eps, fi, time)))
    // We choose to apply the Dirichlet condition for the upwind element
    return std::get<0>(elemIsUpwind(*elem, fi, time));

  return false;
}

ADReal
BernoulliPressureVariable::getDirichletBoundaryFaceValue(const FaceInfo & fi,
                                                         const Elem * const elem,
                                                         const Moose::StateArg & time) const
{
  mooseAssert(isDirichletBoundaryFace(fi, elem, time), "This better be a Dirichlet face");

  if (INSFVPressureVariable::isDirichletBoundaryFace(fi, elem, time))
    return INSFVPressureVariable::getDirichletBoundaryFaceValue(fi, elem, time);

  const auto [is_jump_face, eps_elem, eps_neighbor] = NS::isPorosityJumpFace(*_eps, fi, time);
#ifndef NDEBUG
  mooseAssert(is_jump_face,
              "If we are not a traditional Dirichlet face, then we must be a jump face");
#else
  libmesh_ignore(is_jump_face);
#endif

  const Moose::FaceArg face_elem{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, &fi.elem()};
  const Moose::FaceArg face_neighbor{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.neighborPtr()};

  const auto [elem_is_upwind, vel_face] = elemIsUpwind(*elem, fi, time);
  const auto & vel_elem = vel_face;
  const auto & vel_neighbor = vel_face;

  const bool fi_elem_is_upwind = elem == &fi.elem() ? elem_is_upwind : !elem_is_upwind;
  const auto & downwind_face = fi_elem_is_upwind ? face_neighbor : face_elem;

  /*
     If a weakly compressible material is used where the density slightly
     depends on pressure. Given that the two-term expansion on Bernoulli faces is
     enabled, we take use the downwind face assuming the continuity (minimal jump) of the
     density. This protects against an infinite recursion between pressure and
     density.
  */
  ADReal rho_elem = (*_rho)(makeElemArg(fi.elemPtr()), time);
  ADReal rho_neighbor = (*_rho)(makeElemArg(fi.neighborPtr()), time);

  const VectorValue<ADReal> interstitial_vel_elem = vel_elem * (1 / eps_elem);
  const VectorValue<ADReal> interstitial_vel_neighbor = vel_neighbor * (1 / eps_neighbor);
  const auto v_dot_n_elem = interstitial_vel_elem * fi.normal();
  const auto v_dot_n_neighbor = interstitial_vel_neighbor * fi.normal();
  const auto bernoulli_vel_chunk_elem = 0.5 * rho_elem * v_dot_n_elem * v_dot_n_elem;
  const auto bernoulli_vel_chunk_neighbor =
      0.5 * rho_neighbor * v_dot_n_neighbor * v_dot_n_neighbor;

  const auto & upwind_bernoulli_vel_chunk =
      fi_elem_is_upwind ? bernoulli_vel_chunk_elem : bernoulli_vel_chunk_neighbor;
  const auto & downwind_bernoulli_vel_chunk =
      fi_elem_is_upwind ? bernoulli_vel_chunk_neighbor : bernoulli_vel_chunk_elem;
  ADReal p_downwind;
  if (_allow_two_term_expansion_on_bernoulli_faces)
    p_downwind = (*this)(downwind_face, time);
  else
    p_downwind = fi_elem_is_upwind ? (*this)(makeElemArg(fi.neighborPtr()), time)
                                   : (*this)(makeElemArg(fi.elemPtr()), time);

  return p_downwind + downwind_bernoulli_vel_chunk - upwind_bernoulli_vel_chunk;
}
