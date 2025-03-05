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

  virtual void addMatrixContribution() override;

  virtual void addRightHandSideContribution() override;

  virtual void initialSetup() override;

private:
  /// Container for the current advected interpolation coefficients on the face to make sure
  /// we don't compute it multiple times for different terms.
  std::pair<Real, Real> _advected_interp_coeffs;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Maps for wall bounded elements
  std::map<const Elem *, bool> _wall_bounded;
};
