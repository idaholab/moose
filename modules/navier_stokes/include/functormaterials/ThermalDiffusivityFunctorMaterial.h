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

/**
 * Computes the thermal diffusivity given the thermal conductivity, specific heat capacity, and
 * fluid density
 */
class ThermalDiffusivityFunctorMaterial : public FunctorMaterial
{
public:
  ThermalDiffusivityFunctorMaterial(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /// thermal conductivity
  const Moose::Functor<ADReal> & _k;

  /// specific heat capacity
  const Moose::Functor<ADReal> & _cp;

  /// density
  const Moose::Functor<ADReal> & _rho;
};
