//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVFluxKernel.h"
#include "RhieChowMassFlux.h"
#include "LinearFVAdvectionDiffusionBC.h"

/**
 * An advection kernel that implements the advection term for the passive scalar transport equation.
 */
class LinearFVScalarAdvection : public LinearFVFluxKernel
{
public:
  static InputParameters validParams();
  LinearFVScalarAdvection(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  virtual void setupFaceData(const FaceInfo * face_info) override;

protected:
  /// The Rhie-Chow user object that provides us with the face velocity
  const RhieChowMassFlux & _mass_flux_provider;

private:
  /// Container for the current advected interpolation coefficients on the face to make sure
  /// we don't compute it multiple times for different terms.
  std::pair<Real, Real> _advected_interp_coeffs;

  /// Container for the velocity on the face which will be reused in the advection term's
  /// matrix and right hand side contribution
  Real _volumetric_face_flux;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;
};
