//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGrayLambertBC.h"
#include "HeatConductionNames.h"
#include "GrayLambertSurfaceRadiationBase.h"

registerMooseObject("HeatTransferApp", LinearFVGrayLambertBC);

InputParameters
LinearFVGrayLambertBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorRobinBCBase::validParams();
  params.addClassDescription(
      "Applies a surface-to-surface Gray-Lambert radiation heat flux boundary "
      "condition to a linear finite-volume energy equation.");
  params.addRequiredParam<MooseFunctorName>(
      "temperature_radiation",
      "Temperature functor used to reconstruct the local surface emission.");
  params.addRequiredParam<MooseFunctorName>(
      "coeff_diffusion",
      "Diffusion coefficient used by the corresponding LinearFVDiffusion kernel.");
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the GrayLambertSurfaceRadiationBase UO");
  params.addParam<bool>(
      "reconstruct_emission",
      true,
      "Flag to apply constant heat flux on sideset or reconstruct emission by T^4 law.");

  return params;
}

LinearFVGrayLambertBC::LinearFVGrayLambertBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorRobinBCBase(parameters),
    _temperature_radiation(getFunctor<Real>("temperature_radiation")),
    _coeff_diffusion(getFunctor<Real>("coeff_diffusion")),
    _glsr_uo(getUserObject<GrayLambertSurfaceRadiationBase>("surface_radiation_object_name")),
    _reconstruct_emission(getParam<bool>("reconstruct_emission"))
{
  // Request the previous state. This assumes the BC acts on the temperature variable
  _var.sys().needSolutionState(1, Moose::SolutionIterationType::Nonlinear);
}

Real
LinearFVGrayLambertBC::getAlpha(Moose::FaceArg /* face */, Moose::StateArg /* state */) const
{
  const Elem * const elem =
      _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
          ? _current_face_info->elemPtr()
          : _current_face_info->neighborPtr();

  mooseAssert(elem, "The boundary face must have an adjacent element.");

  const auto elem_arg = makeElemArg(elem);
  // First order extrapolation to the face
  return -_coeff_diffusion(elem_arg, Moose::previousNonlinearState());
}

Real
LinearFVGrayLambertBC::getBeta(Moose::FaceArg /* face */, Moose::StateArg /* state */) const
{

  if (!_reconstruct_emission)
    return 0.0;

  std::vector<BoundaryID> matching_boundary_ids;

  for (const auto boundary_id : _current_face_info->boundaryIDs())
    if (hasBoundary(boundary_id))
      matching_boundary_ids.push_back(boundary_id);

  mooseAssert(
      matching_boundary_ids.size() == 1,
      "LinearFVGrayLambertBC expected exactly one matching boundary ID on the current face.");

  const BoundaryID boundary_id = matching_boundary_ids.front();
  const Real emissivity = _glsr_uo.getSurfaceEmissivity(boundary_id);

  // Evaluate the radiation temperature on the adjacent element rather than
  // asking the variable for its value on the boundary face.
  const Elem * const elem =
      _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
          ? _current_face_info->elemPtr()
          : _current_face_info->neighborPtr();

  mooseAssert(elem, "The radiating boundary face must have an adjacent element.");

  const auto elem_arg = makeElemArg(elem);

  // First order extrapolation to the face
  const Real lagged_temperature = _temperature_radiation(
      elem_arg, Moose::previousNonlinearState());

  return -emissivity * HeatConduction::Constants::sigma *
         Utility::pow<3>(lagged_temperature);

}

Real
LinearFVGrayLambertBC::getGamma(Moose::FaceArg face, Moose::StateArg /*state*/) const
{
  const auto & all_face_bids = face.fi->boundaryIDs();
  const auto & all_bc_bids = boundaryIDs();
  std::set<BoundaryID> current_bid;
  set_intersection(all_face_bids.begin(),
                   all_face_bids.end(),
                   all_bc_bids.begin(),
                   all_bc_bids.end(),
                   std::inserter(current_bid, current_bid.begin()));
  if (current_bid.size() != 1)
    paramError("boundary",
               std::to_string(current_bid.size()) +
                   " boundaries overlap. This is not currently supported");

  if (!_reconstruct_emission)
    return _glsr_uo.getSurfaceHeatFluxDensity(*current_bid.begin());

  Real eps = _glsr_uo.getSurfaceEmissivity(*current_bid.begin());
  const auto gamma = -eps * _glsr_uo.getSurfaceIrradiation((*current_bid.begin()));
  return gamma;
}
