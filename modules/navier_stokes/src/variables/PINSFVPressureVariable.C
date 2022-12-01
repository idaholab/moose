//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVPressureVariable.h"
#include "NS.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", PINSFVPressureVariable);

InputParameters
PINSFVPressureVariable::validParams()
{
  auto params = INSFVPressureVariable::validParams();
  params += FunctorInterface::validParams();
  params.addRequiredParam<MooseFunctorName>("u", "The x-component of velocity");
  params.addParam<MooseFunctorName>("v", 0, "The y-component of velocity");
  params.addParam<MooseFunctorName>("w", 0, "The z-component of velocity");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "The porosity");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density");
  return params;
}

PINSFVPressureVariable::PINSFVPressureVariable(const InputParameters & params)
  : INSFVPressureVariable(params),
    FunctorInterface(this),
    _u(nullptr),
    _v(nullptr),
    _w(nullptr),
    _eps(nullptr),
    _rho(nullptr)
{
}

void
PINSFVPressureVariable::initialSetup()
{
  _u = &getFunctor<ADReal>("u", _subproblem);
  _v = &getFunctor<ADReal>("v", _subproblem);
  _w = &getFunctor<ADReal>("w", _subproblem);
  _eps = &getFunctor<ADReal>(NS::porosity, _subproblem);
  _rho = &getFunctor<ADReal>(NS::density, _subproblem);
}

std::tuple<bool, ADRealVectorValue, ADRealVectorValue>
PINSFVPressureVariable::elemIsUpwind(const Elem & elem, const FaceInfo & fi) const
{
  const Moose::SingleSidedFaceArg ssf_elem{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.elem().subdomain_id()};
  const Moose::SingleSidedFaceArg ssf_neighbor{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.neighbor().subdomain_id()};

  const VectorValue<ADReal> vel_elem{(*_u)(ssf_elem), (*_v)(ssf_elem), (*_w)(ssf_elem)};
  const VectorValue<ADReal> vel_neighbor{
      (*_u)(ssf_neighbor), (*_v)(ssf_neighbor), (*_w)(ssf_neighbor)};
  const bool fi_elem_is_upwind = (vel_elem + vel_neighbor) * fi.normal() > 0;
  const bool elem_is_upwind = &elem == &fi.elem() ? fi_elem_is_upwind : !fi_elem_is_upwind;

  return {elem_is_upwind, vel_elem, vel_neighbor};
}

std::pair<bool, const Elem *>
PINSFVPressureVariable::isExtrapolatedBoundaryFace(const FaceInfo & fi) const
{
  const bool extrapolated = [this, &fi]()
  {
    if (isDirichletBoundaryFace(fi))
      return false;
    if (&fi == _ssf_face)
      return true;
    if (!isInternalFace(fi))
      // We are neither a Dirichlet nor an internal face
      return true;

    // If we got here, then we're definitely on an internal face

    if (_green_gauss_elem && std::get<0>(NS::isPorosityJumpFace(*_eps, fi)))
      // We choose to extrapolate for the element that is downwind
      return !std::get<0>(elemIsUpwind(*_green_gauss_elem, fi));

    return false;
  }();

  if (!fi.neighborPtr())
    return std::make_pair(extrapolated, &fi.elem());

  const bool defined_on_elem = this->hasBlocks(fi.elem().subdomain_id());
#ifndef NDEBUG
  const bool defined_on_neighbor = this->hasBlocks(fi.neighbor().subdomain_id());

  mooseAssert(defined_on_elem || defined_on_neighbor,
              "This shouldn't be called if we aren't defined on either side.");
#endif
  const Elem * const ret_elem = defined_on_elem ? &fi.elem() : fi.neighborPtr();
  return std::make_pair(extrapolated, ret_elem);
}

bool
PINSFVPressureVariable::isDirichletBoundaryFace(const FaceInfo & fi) const
{
  if (INSFVPressureVariable::isDirichletBoundaryFace(fi))
    return true;

  if (isInternalFace(fi) && _green_gauss_elem && std::get<0>(NS::isPorosityJumpFace(*_eps, fi)))
    // We choose to apply the Dirichlet condition for the upwind element
    return std::get<0>(elemIsUpwind(*_green_gauss_elem, fi));

  return false;
}

ADReal
PINSFVPressureVariable::getDirichletBoundaryFaceValue(const FaceInfo & fi) const
{
  mooseAssert(isDirichletBoundaryFace(fi), "This better be a Dirichlet face");
  const auto [is_jump_face, eps_elem, eps_neighbor] = NS::isPorosityJumpFace(*_eps, fi);

  if (!is_jump_face)
    return INSFVPressureVariable::getDirichletBoundaryFaceValue(fi);

  const Moose::SingleSidedFaceArg ssf_elem{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.elem().subdomain_id()};
  const Moose::SingleSidedFaceArg ssf_neighbor{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.neighbor().subdomain_id()};

  // For incompressible or weakly compressible flow these should really be about the same
  const auto rho_elem = (*_rho)(ssf_elem), rho_neighbor = (*_rho)(ssf_neighbor);

  mooseAssert(_green_gauss_elem, "If we are here, then we must have a Green-Gauss element");
  const auto [gg_elem_is_upwind, vel_elem, vel_neighbor] = elemIsUpwind(*_green_gauss_elem, fi);

  const VectorValue<ADReal> interstitial_vel_elem = vel_elem * (1 / eps_elem);
  const VectorValue<ADReal> interstitial_vel_neighbor = vel_neighbor * (1 / eps_neighbor);
  const auto v_dot_n_elem = interstitial_vel_elem * fi.normal();
  const auto v_dot_n_neighbor = interstitial_vel_neighbor * fi.normal();
  const auto bernoulli_vel_chunk_elem = 0.5 * rho_elem * v_dot_n_elem * v_dot_n_elem;
  const auto bernoulli_vel_chunk_neighbor =
      0.5 * rho_neighbor * v_dot_n_neighbor * v_dot_n_neighbor;

  const bool fi_elem_is_upwind =
      _green_gauss_elem == &fi.elem() ? gg_elem_is_upwind : !gg_elem_is_upwind;
  const auto & downwind_ssf = fi_elem_is_upwind ? ssf_neighbor : ssf_elem;
  const auto & upwind_bernoulli_vel_chunk =
      fi_elem_is_upwind ? bernoulli_vel_chunk_elem : bernoulli_vel_chunk_neighbor;
  const auto & downwind_bernoulli_vel_chunk =
      fi_elem_is_upwind ? bernoulli_vel_chunk_neighbor : bernoulli_vel_chunk_elem;
  const auto p_downwind = (*this)(downwind_ssf);
  return p_downwind + downwind_bernoulli_vel_chunk - upwind_bernoulli_vel_chunk;
}

const VectorValue<ADReal> &
PINSFVPressureVariable::adGradSln(const Elem * const elem, const bool correct_skewness) const
{
  _green_gauss_elem = elem;
  const auto & grad = INSFVPressureVariable::adGradSln(elem, correct_skewness);
  _green_gauss_elem = nullptr;
  return grad;
}
