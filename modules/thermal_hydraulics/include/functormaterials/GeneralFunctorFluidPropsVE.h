//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Functor material to access the local fluid properties as functors.
 */
class GeneralFunctorFluidPropsVE : public FunctorMaterial
{
public:
  static InputParameters validParams();

  GeneralFunctorFluidPropsVE(const InputParameters & parameters);

protected:
  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;

  /// specific internal energy
  const Moose::Functor<ADReal> * _e;
  /// specific volume
  const Moose::Functor<ADReal> * _v;

  /// density times area
  const Moose::Functor<ADReal> * const _rhoA;
  /// density times area times energy
  const Moose::Functor<ADReal> * const _rhoEA;
  /// momentum times area
  const Moose::Functor<ADReal> * const _rhouA;
  /// area
  const Moose::Functor<ADReal> * const _A;
  /// local phase fraction
  const Moose::Functor<ADReal> * const _alpha;
};
