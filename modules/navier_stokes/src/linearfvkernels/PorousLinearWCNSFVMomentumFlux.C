//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousLinearWCNSFVMomentumFlux.h"
#include "LinearFVAdvectionDiffusionBC.h"
#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", PorousLinearWCNSFVMomentumFlux);

InputParameters
PorousLinearWCNSFVMomentumFlux::validParams()
{
  InputParameters params = LinearWCNSFVMomentumFlux::validParams();
  params.addClassDescription(
      "Momentum flux kernel with porous-specific advection handling.");
  params.addParam<bool>(
      "porosity_outside_divergence",
      false,
      "Multiply the advection term by porosity outside the divergence operator "
      "(i.e. do not scale the advected interpolation by 1/eps).");
  return params;
}

PorousLinearWCNSFVMomentumFlux::PorousLinearWCNSFVMomentumFlux(const InputParameters & params)
  : LinearWCNSFVMomentumFlux(params),
    _porosity_outside_divergence(getParam<bool>("porosity_outside_divergence"))
{
}

void
PorousLinearWCNSFVMomentumFlux::setupFaceData(const FaceInfo * face_info)
{
  LinearWCNSFVMomentumFlux::setupFaceData(face_info);

  _advected_interp_coeffs = _mass_flux_provider.getAdvectedInterpolationCoeffs(
      *_current_face_info,
      _advected_interp_method,
      _face_mass_flux,
      /*apply_porosity_scaling=*/!_porosity_outside_divergence);
}

Real
PorousLinearWCNSFVMomentumFlux::computeAdvectionBoundaryMatrixContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_matrix_contrib = bc->computeBoundaryValueMatrixContribution();
  const bool elem_side = (_current_face_type != FaceInfo::VarFaceNeighbors::NEIGHBOR);
  const Real eps = _mass_flux_provider.getFaceSidePorosity(*_current_face_info,
                                                           elem_side,
                                                           determineState());
  const Real scale = _porosity_outside_divergence ? 1.0 : 1.0 / eps;
  return boundary_value_matrix_contrib * _face_mass_flux * scale;
}

Real
PorousLinearWCNSFVMomentumFlux::computeAdvectionBoundaryRHSContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_rhs_contrib = bc->computeBoundaryValueRHSContribution();
  const bool elem_side = (_current_face_type != FaceInfo::VarFaceNeighbors::NEIGHBOR);
  const Real eps = _mass_flux_provider.getFaceSidePorosity(*_current_face_info,
                                                           elem_side,
                                                           determineState());
  const Real scale = _porosity_outside_divergence ? 1.0 : 1.0 / eps;
  return -boundary_value_rhs_contrib * _face_mass_flux * scale;
}
