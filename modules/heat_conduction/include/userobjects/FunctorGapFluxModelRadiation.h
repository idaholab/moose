//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelRadiationBase.h"

/**
 * Gap flux model for heat conduction across a gap due to radiation, based on the diffusion
 * approximation. Uses a temperature functor to provide the arguments to the \p computeRadiationFlux
 * method in the base class
 */
class FunctorGapFluxModelRadiation : public GapFluxModelRadiationBase
{
public:
  static InputParameters validParams();

  FunctorGapFluxModelRadiation(const InputParameters & parameters);

  ADReal computeFlux() const override;

protected:
  /// temperature functor for computing temperature along the secondary and primary surfaces
  const Moose::Functor<ADReal> & _T;
};
