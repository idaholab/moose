//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVFluxKernel.h"
#include "MathFVUtils.h"

class ConservativeDiffuseInterfaceMixtureEnthalpy;

/**
 * Heat conduction -div(k grad T) in the equation for the conserved mixture enthalpy density rho*h
 * of a diffuse-interface flow. The two-point face gradient of T is written in rho*h through the
 * per-cell linearization T = rho*h / C + (T_lag - (rho*h)^* / C), with C = d(rho*h)/dT, so the
 * conduction is implicit in rho*h and vanishes exactly at a uniform temperature. The cell
 * conductivity is the series mix of the phase conductivities at the lagged temperature. No
 * nonorthogonal correction is applied.
 */
class LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion : public LinearFVFluxKernel
{
public:
  static InputParameters validParams();
  LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion(const InputParameters & params);

  void initialSetup() override;

  Real computeElemMatrixContribution() override { return _elem_coeff * _current_face_area; }
  Real computeNeighborMatrixContribution() override { return _neighbor_coeff * _current_face_area; }
  Real computeElemRightHandSideContribution() override
  {
    return -_flux_constant * _current_face_area;
  }
  Real computeNeighborRightHandSideContribution() override
  {
    return _flux_constant * _current_face_area;
  }
  Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;
  Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  void setupFaceData(const FaceInfo * face_info) override;

private:
  const ConservativeDiffuseInterfaceMixtureEnthalpy & _mixture;
  /// Interpolation of the cell conductivities to an internal face
  Moose::FV::InterpMethod _conductivity_interp_method;

  /// Coefficients of the element and neighbor rho*h in the conductive flux density leaving the
  /// element. They are per unit face area because the face loop sets the area only after
  /// setupFaceData().
  Real _elem_coeff;
  Real _neighbor_coeff;
  /// The part of the conductive flux density leaving the element that does not depend on the
  /// solution
  Real _flux_constant;
};
