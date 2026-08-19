//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Computes the source and sink terms for the thermal radiation
 */
class FVSP3ThermalRadiationSourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  FVSP3ThermalRadiationSourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// Temperature
  const Moose::Functor<ADReal> & _T;

  /// Frequency
  const Real _nu;

  /// Frequency bounds for numerical integration
  const Real * _nu_low;
  const Real * _nu_high;

  /// Refraction index
  const Moose::Functor<ADReal> & _n1;

  /// kappa Thickness
  const Moose::Functor<ADReal> & _absorptivity;

  /// Units of Constants
  const std::string _planck_units;
  const std::string _sol_units;
  const std::string _boltzmann_units;
};
