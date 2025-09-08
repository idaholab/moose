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
class FVSP3ThermalRadiationBC : public FVFluxBC
{
public:
  FVSP3ThermalRadiationBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// Temperature at the boundary
  const Moose::Functor<ADReal> & _Tb;

  /// Frequency of thermal radiation band
  const Real _nu;

  /// Frequency bounds of thermal radiation band for numerical integration
  const Real * _nu_low;
  const Real * _nu_high;

  /// Refraction index
  const Moose::Functor<ADReal> & _n1;

  /// Optical Thickness of medium
  const Moose::Functor<ADReal> & _optical_thickness;

  /// Radiation function from the conjugated other
  const Moose::Functor<ADReal> & _psi;

  /// Order or radiation moment
  const MooseEnum _order;

  /// Coefficients for SP3
  const Moose::Functor<ADReal> & _alpha;
  const Moose::Functor<ADReal> & _beta;
  const Moose::Functor<ADReal> & _eta;

  /// Governing parameters for SP3 moments
  Real _squared_mu_order;

  /// Closure parameters
  const Real _squared_mu_1 = 3. / 7. - 2. / 7. * std::sqrt(6. / 5.);
  const Real _squared_mu_2 = 3. / 7. + 2. / 7. * std::sqrt(6. / 5.);

  /// Units of Constants
  const std::string _planck_units;
  const std::string _sol_units;
  const std::string _boltzmann_units;
};
