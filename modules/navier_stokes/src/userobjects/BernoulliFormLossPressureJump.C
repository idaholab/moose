//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "BernoulliFormLossPressureJump.h"

#include "FVUtils.h"
#include "MooseEnum.h"

#include <cmath>

registerMooseObject("NavierStokesApp", BernoulliFormLossPressureJump);

InputParameters
BernoulliFormLossPressureJump::validParams()
{
  auto params = PressureJumpModel::validParams();
  params.addClassDescription(
      "Provides Bernoulli and form-loss pressure jumps to PorousRhieChowMassFlux.");
  params.addRequiredParam<MooseFunctorName>("porosity", "The porosity functor.");
  params.addRequiredParam<MooseFunctorName>("density", "The density functor.");
  params.addParam<std::vector<Real>>(
      "form_loss", {}, "Form-loss coefficients aligned with the pressure-jump boundaries.");
  MultiMooseEnum reference_velocity_side("lower_porosity higher_porosity");
  params.addParam<MultiMooseEnum>(
      "reference_velocity_side",
      reference_velocity_side,
      "Reference-velocity sides aligned with the pressure-jump boundaries. The default is "
      "lower_porosity.");
  params.addParam<bool>("use_interpolated_density",
                        false,
                        "Use the face-interpolated density in the reversible Bernoulli term "
                        "instead of the owner and non-owner densities.");
  return params;
}

BernoulliFormLossPressureJump::BernoulliFormLossPressureJump(const InputParameters & params)
  : PressureJumpModel(params),
    _porosity(getFunctor<Real>("porosity")),
    _density(getFunctor<Real>("density")),
    _use_interpolated_density(getParam<bool>("use_interpolated_density"))
{
  const auto & boundary_names = getParam<std::vector<BoundaryName>>("boundary");
  const auto & form_loss = getParam<std::vector<Real>>("form_loss");
  const auto & reference_velocity_side = getParam<MultiMooseEnum>("reference_velocity_side");

  if (!form_loss.empty() && form_loss.size() != boundary_names.size())
    paramError("form_loss", "Must have the same length as boundary.");
  if (params.isParamSetByUser("reference_velocity_side") &&
      reference_velocity_side.size() != boundary_names.size())
    paramError("reference_velocity_side", "Must have the same length as boundary.");
  if (params.isParamSetByUser("reference_velocity_side") && form_loss.empty())
    paramError("reference_velocity_side", "Requires form_loss to be provided.");

  for (const auto value : form_loss)
    if (value < 0.0)
      paramError("form_loss", "All entries must be >= 0.");

  for (const auto i : index_range(boundary_names))
  {
    const auto boundary_id = _moose_mesh.getBoundaryID(boundary_names[i]);
    _form_loss_by_id[boundary_id] = form_loss.empty() ? 0.0 : form_loss[i];
    _use_higher_porosity_by_id[boundary_id] = params.isParamSetByUser("reference_velocity_side")
                                                  ? reference_velocity_side[i] == "higher_porosity"
                                                  : false;
  }
}

Real
BernoulliFormLossPressureJump::computePressureJump(const FaceInfo & fi,
                                                   const Real face_mass_flux) const
{
  mooseAssert(appliesTo(fi), "The pressure jump model must apply to the supplied face.");
  mooseAssert(fi.neighborPtr(), "A pressure jump requires an internal face.");

  const auto time = Moose::currentState();
  const auto elem_rho = _density(makeElemArg(fi.elemPtr()), time);
  const auto neighbor_rho = _density(makeElemArg(fi.neighborPtr()), time);
  const auto elem_porosity = _porosity(makeElemArg(fi.elemPtr()), time);
  const auto neighbor_porosity = _porosity(makeElemArg(fi.neighborPtr()), time);

  if (elem_rho <= 0.0 || neighbor_rho <= 0.0)
    mooseError(name(), ": density must be positive on pressure-jump face ", fi.id(), ".");
  if (elem_porosity <= 0.0 || neighbor_porosity <= 0.0)
    mooseError(name(), ": porosity must be positive on pressure-jump face ", fi.id(), ".");

  Real face_rho = 0.0;
  Moose::FV::interpolate(
      Moose::FV::InterpMethod::Average, face_rho, elem_rho, neighbor_rho, fi, true);

  const bool elem_is_owner = elemIsOwner(fi);
  const Real rho_owner = elem_is_owner ? elem_rho : neighbor_rho;
  const Real rho_non_owner = elem_is_owner ? neighbor_rho : elem_rho;
  const Real porosity_owner = elem_is_owner ? elem_porosity : neighbor_porosity;
  const Real porosity_non_owner = elem_is_owner ? neighbor_porosity : elem_porosity;
  Real pressure_jump;

  // The reversible term is the difference in dynamic pressure between the two sides.
  if (_use_interpolated_density)
  {
    const Real superficial_velocity = face_mass_flux / face_rho;
    const Real velocity_owner = superficial_velocity / porosity_owner;
    const Real velocity_non_owner = superficial_velocity / porosity_non_owner;
    pressure_jump = 0.5 * face_rho *
                    (velocity_owner * velocity_owner - velocity_non_owner * velocity_non_owner);
  }
  else
  {
    const Real velocity_owner = face_mass_flux / (rho_owner * porosity_owner);
    const Real velocity_non_owner = face_mass_flux / (rho_non_owner * porosity_non_owner);
    pressure_jump = 0.5 * (rho_owner * velocity_owner * velocity_owner -
                           rho_non_owner * velocity_non_owner * velocity_non_owner);
  }

  BoundaryID boundary_id = Moose::INVALID_BOUNDARY_ID;
  for (const auto candidate_id : fi.boundaryIDs())
    if (boundaryIDs().count(candidate_id))
    {
      boundary_id = candidate_id;
      break;
    }

  const Real form_loss = libmesh_map_find(_form_loss_by_id, boundary_id);
  if (form_loss > 0.0 && face_mass_flux != 0.0)
  {
    const bool use_higher_porosity = libmesh_map_find(_use_higher_porosity_by_id, boundary_id);
    const bool use_elem = use_higher_porosity ? elem_porosity >= neighbor_porosity
                                              : elem_porosity <= neighbor_porosity;
    const Real reference_velocity = use_elem ? face_mass_flux / (elem_rho * elem_porosity)
                                             : face_mass_flux / (neighbor_rho * neighbor_porosity);
    const Real owner_to_non_owner_mass_flux =
        elem_is_owner ? face_mass_flux : -face_mass_flux;
    const Real flow_sign = owner_to_non_owner_mass_flux > 0.0 ? 1.0 : -1.0;
    pressure_jump -=
        flow_sign * 0.5 * form_loss * face_rho * reference_velocity * reference_velocity;
  }

  return pressure_jump;
}

bool
BernoulliFormLossPressureJump::useOneSidedReconstruction(const FaceInfo & fi) const
{
  mooseAssert(appliesTo(fi), "The pressure jump model must apply to the supplied face.");
  if (!fi.neighborPtr())
    return false;

  const auto time = Moose::currentState();
  const Real elem_porosity = _porosity(makeElemArg(fi.elemPtr()), time);
  const Real neighbor_porosity = _porosity(makeElemArg(fi.neighborPtr()), time);
  const bool porosity_jump = std::abs(elem_porosity - neighbor_porosity) > TOLERANCE;

  return porosity_jump || !_use_interpolated_density;
}
