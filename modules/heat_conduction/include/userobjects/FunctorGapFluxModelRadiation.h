//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelBase.h"

/**
 * Base class for gap flux models used by ModularGapConductanceConstraint
 */
class FunctorGapFluxModelRadiation : public GapFluxModelBase
{
public:
  static InputParameters validParams();

  FunctorGapFluxModelRadiation(const InputParameters & parameters);

  ADReal computeFlux() const override;

protected:
  /// temperature functor for computing temperature along the secondary and primary surfaces
  const Moose::Functor<ADReal> & _T;

  /// Stefan-Boltzmann constant
  const Real _stefan_boltzmann;

  /// Surface emissivity
  ADReal _emissivity;
};
