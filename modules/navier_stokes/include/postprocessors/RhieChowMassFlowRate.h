//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"
#include "RhieChowMassFlux.h"
#include "MathFVUtils.h"

/**
 * Integrates the mass flux stored by a linear segregated Rhie-Chow user object, optionally
 * weighted by an advected quantity.
 */
class RhieChowMassFlowRate : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  RhieChowMassFlowRate(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
  virtual Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  const RhieChowMassFlux & _mass_flux_provider;

  /// Optional advected quantity to multiply by the Rhie-Chow mass flux
  const Moose::Functor<ADReal> * const _adv_quant;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;
};
