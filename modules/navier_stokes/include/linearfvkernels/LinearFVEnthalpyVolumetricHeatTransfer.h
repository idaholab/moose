//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVVolumetricHeatTransfer.h"

/**
 * Volumetric heat transfer kernel for enthalpy equations using an auxiliary temperature together
 * with a local dT/dh ~= 1/cp linearization on the current side.
 */
class LinearFVEnthalpyVolumetricHeatTransfer : public LinearFVVolumetricHeatTransfer
{
public:
  static InputParameters validParams();

  LinearFVEnthalpyVolumetricHeatTransfer(const InputParameters & params);

  Real computeMatrixContribution() override;
  Real computeRightHandSideContribution() override;

protected:
  /// Specific heat used for the current-side temperature linearization
  const Moose::Functor<Real> & _cp;
};
