//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConservativeDiffuseInterfaceRhieChowMassFluxBase.h"

/**
 * Diffuse-interface Rhie-Chow implementation for the velocity-component momentum path.
 *
 * The momentum unknowns are velocity components U_i. Conservative behavior is supplied by the
 * density-weighted matrix coefficients and face fluxes for the reduced-pressure
 * ddt(rho, U) + div(rhoPhi, U) form without storing rho*U as a primary unknown.
 */
class ConservativeDiffuseInterfaceRhieChowMassFlux
  : public ConservativeDiffuseInterfaceRhieChowMassFluxBase
{
public:
  static InputParameters validParams();

  ConservativeDiffuseInterfaceRhieChowMassFlux(const InputParameters & params);

  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) const override;
};
