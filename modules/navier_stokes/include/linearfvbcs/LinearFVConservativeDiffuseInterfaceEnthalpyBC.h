//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVBoundaryCondition.h"

class ConservativeDiffuseInterfaceMixtureEnthalpy;

/**
 * Boundary condition on the conserved mixture enthalpy density rho*h of a diffuse-interface flow,
 * given as a temperature or as a heat flux. With a temperature, it is the temperature of the
 * fluid entering the domain and the face temperature for conduction. With a heat flux, entering
 * fluid carries the temperature of the adjacent cell.
 */
class LinearFVConservativeDiffuseInterfaceEnthalpyBC : public LinearFVBoundaryCondition
{
public:
  static InputParameters validParams();

  LinearFVConservativeDiffuseInterfaceEnthalpyBC(const InputParameters & parameters);

  /// rho*h on the face: of the boundary temperature with a temperature, of the cell otherwise
  Real computeBoundaryValue() const override;

  Real computeBoundaryNormalGradient() const override;

  /// Whether the face temperature is prescribed
  bool hasTemperature() const { return _temperature; }

  /// The prescribed face temperature
  Real boundaryTemperature() const;

  /// The heat flux into the domain
  Real heatFlux() const;

  /// The distance from the cell center to the face
  Real cellToFaceDistance() const { return computeCellToFaceDistance(); }

private:
  const ConservativeDiffuseInterfaceMixtureEnthalpy & _mixture;
  const Moose::Functor<Real> & _alpha;
  /// The prescribed boundary temperature, null where a heat flux is prescribed instead
  const Moose::Functor<Real> * const _temperature;
  const Moose::Functor<Real> & _heat_flux;
};
