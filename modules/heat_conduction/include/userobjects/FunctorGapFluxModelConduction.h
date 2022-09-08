//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelConductionBase.h"

/**
 * Gap flux model for varying gap conductance using a functor for temperature
 */
class FunctorGapFluxModelConduction : public GapFluxModelConductionBase
{
public:
  static InputParameters validParams();

  FunctorGapFluxModelConduction(const InputParameters & parameters);

  virtual ADReal computeFlux() const override;

protected:
  /// temperature functor for computing temperature along the secondary and primary surfaces
  const Moose::Functor<ADReal> & _T;

  /// Thermal conductivity multiplier. Multiplied by the constant gap_conductivity to form the
  /// final conductivity
  const Moose::Functor<ADReal> & _gap_conductivity_multiplier;
};
