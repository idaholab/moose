//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVVOFCompression.h"

#include "MooseFunctorArguments.h"

#include <algorithm>
#include <cmath>

registerMooseObject("NavierStokesApp", LinearFVVOFCompression);

InputParameters
LinearFVVOFCompression::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription(
      "Adds an explicit conservative interface-compression flux to a linear-FV volume-fraction "
      "transport equation.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The Rhie-Chow user object used to obtain the physical volumetric face flux.");
  params.addParam<MooseFunctorName>(
      "compression_factor",
      "0",
      "Compression coefficient c_alpha used to scale the explicit compressive face flux.");
  params.addRequiredParam<MooseFunctorName>(
      "interface_normal",
      "Face-oriented interface unit normal used to align the compressive flux.");
  return params;
}

LinearFVVOFCompression::LinearFVVOFCompression(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _compression_factor(getFunctor<Real>("compression_factor")),
    _interface_normal(getFunctor<RealVectorValue>("interface_normal")),
    _compression_flux(0.0)
{
}

Real
LinearFVVOFCompression::computeElemMatrixContribution()
{
  return 0.0;
}

Real
LinearFVVOFCompression::computeNeighborMatrixContribution()
{
  return 0.0;
}

Real
LinearFVVOFCompression::computeElemRightHandSideContribution()
{
  return computeCompressionFlux();
}

Real
LinearFVVOFCompression::computeNeighborRightHandSideContribution()
{
  return -computeCompressionFlux();
}

Real
LinearFVVOFCompression::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & /*bc*/)
{
  return 0.0;
}

Real
LinearFVVOFCompression::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & /*bc*/)
{
  return 0.0;
}

void
LinearFVVOFCompression::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);
  _compression_flux = 0.0;
}

Real
LinearFVVOFCompression::computeCompressionFlux()
{
  if (_current_face_type != FaceInfo::VarFaceNeighbors::BOTH)
    return 0.0;

  if (_cached_rhs_contribution)
    return _compression_flux;

  const auto state = determineState();
  const auto face_arg = makeCDFace(*_current_face_info);

  const Real elem_alpha = clampAlpha(_var.getElemValue(*_current_face_info->elemInfo(), state));
  const Real neighbor_alpha =
      clampAlpha(_var.getElemValue(*_current_face_info->neighborInfo(), state));
  const Real alpha_face =
      clampAlpha(_current_face_info->gC() * elem_alpha +
                 (1.0 - _current_face_info->gC()) * neighbor_alpha);

  const RealVectorValue face_normal = _current_face_info->normal();
  const Real face_normal_mag = face_normal.norm();
  const RealVectorValue face_unit_normal =
      face_normal_mag > 0.0 ? face_normal / face_normal_mag : RealVectorValue();
  const RealVectorValue interface_normal =
      MetaPhysicL::raw_value(_interface_normal(face_arg, state));

  const Real physical_face_flux = _mass_flux_provider.getVolumetricFaceFlux(*_current_face_info);
  const Real compressive_speed =
      std::abs(physical_face_flux) *
      MetaPhysicL::raw_value(_compression_factor(face_arg, state)) *
      (interface_normal * face_unit_normal);

  _cached_rhs_contribution = true;
  _compression_flux = compressive_speed * alpha_face * (1.0 - alpha_face) * _current_face_area;
  return _compression_flux;
}

Real
LinearFVVOFCompression::clampAlpha(const Real value)
{
  return std::max(0.0, std::min(1.0, value));
}
