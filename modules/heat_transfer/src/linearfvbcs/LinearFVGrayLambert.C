//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGrayLambert.h"
#include "HeatConductionNames.h"
#include "GrayLambertSurfaceRadiationBase.h"


registerMooseObject("HeatTransferApp", LinearFVGrayLambert);

InputParameters
LinearFVGrayLambert::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorRobinBCBase::validParams();
  params.addClassDescription("Marshak boundary condition for radiative heat flux.");
  params.addRequiredParam<MooseFunctorName>("temperature_radiation", "The radiation temperature.");
  params.addRequiredParam<MooseFunctorName>("coeff_diffusion",
                                            "Radiative heat flux P1 diffusion coefficient.");
  // params.addParam<MooseFunctorName>("boundary_emissivity", 1.0, "Emissivity of the boundary.");
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the GrayLambertSurfaceRadiationBase UO");

  return params;
}

LinearFVGrayLambert::LinearFVGrayLambert(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorRobinBCBase(parameters),
    _temperature_radiation(getFunctor<Real>("temperature_radiation")),
    _coeff_diffusion(getFunctor<Real>("coeff_diffusion")),
    // _eps_boundary(getFunctor<Real>("boundary_emissivity")),
    _glsr_uo(getUserObject<GrayLambertSurfaceRadiationBase>("surface_radiation_object_name"))
{
}

Real
LinearFVGrayLambert::getAlpha(Moose::FaceArg face, Moose::StateArg state) const
{
  const auto alpha = -_coeff_diffusion(face, state);
  return alpha;
}

Real
LinearFVGrayLambert::getBeta(Moose::FaceArg face, Moose::StateArg state) const
{
  const auto & all_face_bids = _current_face_info->boundaryIDs();
  const auto & all_bc_bids = boundaryIDs();
  std::set<BoundaryID> current_bid;
  set_intersection(all_face_bids.begin(), all_face_bids.end(), all_bc_bids.begin(), all_bc_bids.end(),
                 std::inserter(current_bid, current_bid.begin()));
  if (current_bid.size() != 1)
    paramError("boundary", std::to_string(current_bid.size()) + " boundaries overlap. This is not currently supported");

  // std::cout << "Normal: " <<_current_face_info->normal() << " Temp: " << _temperature_radiation(face, state) << std::endl;
  // std::cout << " Temp: " << _temperature_radiation(face, state) << " current_bid: " << *current_bid.begin() << std::endl;
  Real eps = _glsr_uo.getSurfaceEmissivity(*current_bid.begin());

  const auto beta = -eps * HeatConduction::Constants::sigma *
                       Utility::pow<3>(_temperature_radiation(face, state));
  return beta;
}

Real
LinearFVGrayLambert::getGamma(Moose::FaceArg face, Moose::StateArg /*state*/) const
{
  const auto & all_face_bids = face.fi->boundaryIDs();
  const auto & all_bc_bids = boundaryIDs();
  std::set<BoundaryID> current_bid;
  set_intersection(all_face_bids.begin(), all_face_bids.end(), all_bc_bids.begin(), all_bc_bids.end(),
                 std::inserter(current_bid, current_bid.begin()));
  if (current_bid.size() != 1)
    paramError("boundary", std::to_string(current_bid.size()) + " boundaries overlap. This is not currently supported");

  Real eps = _glsr_uo.getSurfaceEmissivity(*current_bid.begin());

  const auto gamma = -eps * _glsr_uo.getSurfaceIrradiation((*current_bid.begin()));

  return gamma;
}
