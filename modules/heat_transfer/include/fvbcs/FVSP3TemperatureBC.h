//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Robin boundary condition (temperatures) for finite volume scheme between
 * a solid and fluid where the temperatures and heat transfer coefficient
 * are given as a functors
 */
class FVSP3TemperatureBC : public FVFluxBC
{
public:
  FVSP3TemperatureBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// Boundary Temperature
  const Moose::Functor<ADReal> & _Tb;

  /// Refraction index
  const Moose::Functor<ADReal> & _n1;

  /// Refraction index 2
  const Moose::Functor<ADReal> & _n2;

  /// Convective heat transfer coefficient of medium
  const Moose::Functor<ADReal> & _h;

  /// Conductivitiy of medium
  const Moose::Functor<ADReal> & _k;

  /// Optical thickness of medium
  const Moose::Functor<ADReal> & _epsilon;

  /// hemispheric emissivity
  const Moose::Functor<ADReal> & _alpha;

  /// Opaque frequency of medium
  const Real _nu1;

  /// Minimum frequency
  const Real _nu_min;

  /// Units of Constants
  const std::string _planck_units;
  const std::string _sol_units;
  const std::string _boltzmann_units;
};
