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
class FunctorGapFluxModelConduction : public GapFluxModelBase
{
public:
  static InputParameters validParams();

  FunctorGapFluxModelConduction(const InputParameters & parameters);

  virtual ADReal computeFlux() const override;

  virtual ADReal gapAttenuation() const;

protected:
  /// temperature functor for computing temperature along the secondary and primary surfaces
  const Moose::Functor<ADReal> & _T;

  /// Gap conductivity constant
  const Real _gap_conductivity;

  /// Thermal conductivity of the gap material as an ADReal functor.  Multiplied by the constant
  /// gap_conductivity to form the final conductivity
  const Moose::Functor<ADReal> & _gap_conductivity_functor;

  const Real _min_gap;

  const unsigned int _min_gap_order;
};
