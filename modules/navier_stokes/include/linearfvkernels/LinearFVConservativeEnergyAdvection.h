//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "LinearFVFluxKernel.h"
#include "LinearFVAdvectionDiffusionBC.h"

/**
 * Advection of conserved thermal energy E = rho * cp * T using a face flux for rho * cp.
 */
class LinearFVConservativeEnergyAdvection : public LinearFVFluxKernel
{
public:
  static InputParameters validParams();
  LinearFVConservativeEnergyAdvection(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;
  virtual Real computeNeighborMatrixContribution() override;
  virtual Real computeElemRightHandSideContribution() override;
  virtual Real computeNeighborRightHandSideContribution() override;
  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;
  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;
  virtual void setupFaceData(const FaceInfo * face_info) override;

private:
  Real elemInverseRhoCp() const;
  Real neighborInverseRhoCp() const;
  Real singleSidedInverseRhoCp() const;

  /// Face flux of rho * cp, optionally already integrated over face area
  const Moose::Functor<Real> & _rho_cp_face_flux;

  /// Cell-centered rho * cp functor
  const Moose::Functor<Real> & _rho_cp;

  /// Whether the provided face flux already contains face area
  const bool _face_flux_is_integrated;

  /// Interpolation coefficients for the upwinded temperature E / rho_cp
  std::pair<Real, Real> _advected_interp_coeffs;

  /// Face flux per unit area
  Real _face_flux;

  /// The interpolation method to use for the advected temperature
  Moose::FV::InterpMethod _advected_interp_method;
};
