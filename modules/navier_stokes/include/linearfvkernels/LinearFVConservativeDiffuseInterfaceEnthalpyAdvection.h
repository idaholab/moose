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

#include <array>

class ConservativeDiffuseInterfaceMixtureEnthalpy;

/**
 * Advection of the conserved mixture enthalpy density rho*h of a diffuse-interface flow. Each
 * phase carries its own upwind specific enthalpy with its own mass flux: the liquid's is the
 * liquid density times the limited volume-fraction flux, and the gas's is the remainder of the
 * mixture mass flux, both published by the volume-fraction corrector. The face enthalpy
 * h_k(T_upwind) is linearized about the lagged temperature of the upwind cell, with T expressed
 * through rho*h at the current volume fraction, so a uniform temperature is transported exactly.
 *
 * Splitting the flux by phase is what makes the transport consistent: the mass flux of each
 * phase through a face is exactly the mass of that phase the volume-fraction update moved, so
 * the energy that arrives in a cell is the energy of the fluid that arrived. Advecting one
 * mixture enthalpy with the mixture mass flux instead would carry the enthalpy of the wrong phase
 * wherever the two phase fluxes point in different directions or differ in size, which at a
 * sharp interface they routinely do.
 */
class LinearFVConservativeDiffuseInterfaceEnthalpyAdvection : public LinearFVFluxKernel
{
public:
  static InputParameters validParams();
  LinearFVConservativeDiffuseInterfaceEnthalpyAdvection(const InputParameters & params);

  void initialSetup() override;

  Real computeElemMatrixContribution() override { return _elem_coeff; }
  Real computeNeighborMatrixContribution() override { return _neighbor_coeff; }
  Real computeElemRightHandSideContribution() override { return -_flux_constant; }
  Real computeNeighborRightHandSideContribution() override { return _flux_constant; }
  Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;
  Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  void setupFaceData(const FaceInfo * face_info) override;

private:
  /// Adds the linearized flux of one phase leaving the upwind cell through the current face
  void
  addPhaseFlux(unsigned int phase, Real mass_flux, const ElemInfo & upwind, bool upwind_is_elem);

  const ConservativeDiffuseInterfaceMixtureEnthalpy & _mixture;
  /// Area-integrated limited volume-fraction flux, timestep-averaged over the alpha subcycles
  const Moose::Functor<Real> & _alpha_flux;
  /// Area-integrated mixture mass flux consistent with the volume-fraction flux
  const Moose::Functor<Real> & _mass_flux;

  /// Phase mass fluxes through the current face, oriented along the face normal
  std::array<Real, 2> _phase_mass_flux;
  /// Coefficients of the element and neighbor rho*h in the flux leaving the element
  Real _elem_coeff;
  Real _neighbor_coeff;
  /// The part of the flux leaving the element that does not depend on the solution
  Real _flux_constant;
};
