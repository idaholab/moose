//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVScalarAdvection.h"
#include "RhieChowMassFlux.h"
#include "LinearFVAdvectionDiffusionBC.h"

/**
 * An advection kernel that implements the advection term for the enthalpy in the
 * energy equation.
 */
class LinearFVTurbulentLimitedAdvection : public LinearFVScalarAdvection
{
public:
  static InputParameters validParams();
  LinearFVTurbulentLimitedAdvection(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  virtual void initialSetup() override;

private:
  /// Container for the current advected interpolation coefficients on the face to make sure
  /// we don't compute it multiple times for different terms.
  std::pair<Real, Real> _advected_interp_coeffs;

  /// Container for the mass flux on the face which will be reused in the advection term's
  /// matrix and right hand side contribution
  Real _face_mass_flux;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Maps for wall bounded elements
  std::map<const Elem *, bool> _wall_bounded;
};