//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addParam<std::vector<BoundaryName>>(
      "pressure_drop_sidesets", {}, "Sidesets over which form loss coefficients are to be applied");
  params.addParam<std::vector<Real>>(
      "pressure_drop_form_factors",
      {},
      "User-supplied form loss coefficients to be applied over the sidesets listed above");
  params.addParam<bool>(
      "allow_two_term_expansion_on_bernoulli_faces",
      false,
      "Switch to enable the two-term extrapolation on porosity jump faces. "
      "WARNING: This might lead to crushes in parallel runs if porosity jump faces are connected "
      "with one cell (usually corners) due to the insufficient number of ghosted "
      "layers.");

  // Assuming that this variable is used for advection problems, due to the
  // utilization of the pressure gradient in the advecting velocity
  // through the Rhie-Chow interpolation, we have to extend the ghosted layers.
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & /*obj_params*/, InputParameters & rm_params)
      { rm_params.set<unsigned short>("layers") = 3; });
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
    _pressure_drop_sidesets(getParam<std::vector<BoundaryName>>("pressure_drop_sidesets")),
    _pressure_drop_sideset_ids(this->_mesh.getBoundaryIDs(_pressure_drop_sidesets)),
    _pressure_drop_form_factors(getParam<std::vector<Real>>("pressure_drop_form_factors")),
    _allow_two_term_expansion_on_bernoulli_faces(
        getParam<bool>("allow_two_term_expansion_on_bernoulli_faces"))
{
  if (_allow_two_term_expansion_on_bernoulli_faces &&
      _face_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage)
    paramError(
        "allow_two_term_expansion_on_bernoulli_faces",
        "Skewness correction with two term extrapolation on Bernoulli faces is not supported!");

  if (_pressure_drop_sidesets.size() != _pressure_drop_form_factors.size())
    paramError("pressure_drop_sidesets",
               "The number of sidesets and the number of supplied form losses are not equal!");
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
  const Moose::FaceArg face{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};

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

  if (isSeparatorBoundary(fi))
    return true;

  if (std::get<0>(NS::isPorosityJumpFace<ADReal>(*_eps, fi, time)))
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

  if (isInternalFace(fi))
  {
    if (isSeparatorBoundary(fi))
      return false;

    if (std::get<0>(NS::isPorosityJumpFace<ADReal>(*_eps, fi, time)))
      // We choose to apply the Dirichlet condition for the upwind element
      return std::get<0>(elemIsUpwind(*elem, fi, time));
  }

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

  const auto [is_jump_face, eps_elem, eps_neighbor] =
      NS::isPorosityJumpFace<ADReal>(*_eps, fi, time);
#ifndef NDEBUG
  mooseAssert(is_jump_face,
              "If we are not a traditional Dirichlet face, then we must be a jump face");
#else
  libmesh_ignore(is_jump_face);
#endif

  const Moose::FaceArg face_elem{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, &fi.elem(), nullptr};
  const Moose::FaceArg face_neighbor{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.neighborPtr(), nullptr};

  const auto [elem_is_upwind, vel_face] = elemIsUpwind(*elem, fi, time);

  const bool fi_elem_is_upwind = elem == &fi.elem() ? elem_is_upwind : !elem_is_upwind;
  const auto & downwind_face = fi_elem_is_upwind ? face_neighbor : face_elem;

  const auto & upwind_elem = makeElemArg(fi_elem_is_upwind ? fi.elemPtr() : fi.neighborPtr());
  const auto & downwind_elem = makeElemArg(fi_elem_is_upwind ? fi.neighborPtr() : fi.elemPtr());

  const auto & vel_upwind = vel_face;
  const auto & vel_downwind = vel_face;

  const auto & eps_upwind = fi_elem_is_upwind ? eps_elem : eps_neighbor;
  const auto & eps_downwind = fi_elem_is_upwind ? eps_neighbor : eps_elem;

  /*
    If a weakly compressible material is used where the density slightly
    depends on pressure. Given that the two-term expansion on Bernoulli faces is
    enabled, we take use the downwind face assuming the continuity (minimal jump) of the
    density. This protects against an infinite recursion between pressure and
    density.
  */
  const auto & rho_upwind = (*_rho)(upwind_elem, time);
  const auto & rho_downwind = (*_rho)(downwind_elem, time);

  const VectorValue<ADReal> interstitial_vel_upwind = vel_upwind * (1 / eps_upwind);
  const VectorValue<ADReal> interstitial_vel_downwind = vel_downwind * (1 / eps_downwind);

  const auto v_dot_n_upwind = interstitial_vel_upwind * fi.normal();
  const auto v_dot_n_downwind = interstitial_vel_downwind * fi.normal();

  // Iterate through sidesets to see if we have associated irreversible pressure losses
  // or not.
  ADReal factor = 0.0;
  for (const auto & bd_id : fi.boundaryIDs())
    for (const auto i : index_range(_pressure_drop_sideset_ids))
      if (_pressure_drop_sideset_ids[i] == bd_id)
        factor += _pressure_drop_form_factors[i];

  auto upwind_bernoulli_vel_chunk = 0.5 * rho_upwind * v_dot_n_upwind * v_dot_n_upwind;
  auto downwind_bernoulli_vel_chunk = 0.5 * rho_downwind * v_dot_n_downwind * v_dot_n_downwind;

  // We add the additional, irreversible pressure loss here.
  // If it is a contraction we have to use the downwind values, otherwise the upwind values.
  if (eps_downwind < eps_upwind)
    downwind_bernoulli_vel_chunk +=
        0.5 * factor * rho_downwind * v_dot_n_downwind * v_dot_n_downwind;
  else
    upwind_bernoulli_vel_chunk += -0.5 * factor * rho_upwind * v_dot_n_upwind * v_dot_n_upwind;

  ADReal p_downwind;
  if (_allow_two_term_expansion_on_bernoulli_faces)
    p_downwind = (*this)(downwind_face, time);
  else
    p_downwind = (*this)(downwind_elem, time);

  return p_downwind + downwind_bernoulli_vel_chunk - upwind_bernoulli_vel_chunk;
}
