//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"
#include "FVDiffusionInterpolationInterface.h"

/// FVSP3ThermalRadiationDiffusion implements a diffusion term for the SP3 thermal radiation diffusion/
///
///     - strong form: \nabla \cdot (\epsilon^2 \mu_n^2) / \kappa) \nabla u / coef
///
///     - Integration by parts: \int_{A} (\epsilon^2 \mu_n^2) \nabla u / coef \cdot \vec{n} dA
///
class FVSP3ThermalRadiationDiffusion : public FVFluxKernel, public FVDiffusionInterpolationInterface
{
public:
  static InputParameters validParams();
  FVSP3ThermalRadiationDiffusion(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// Optical Thickness
  const Moose::Functor<ADReal> & _optical_thickness;

  /// kappa Thickness
  const Moose::Functor<ADReal> & _absorptivity;

  /// Order
  const MooseEnum _order;

  /// Mu order scaling
  Real _mu_order;

  /// Decides if a geometric arithmetic or harmonic average is used for the
  /// face interpolation of the diffusion coefficient.
  Moose::FV::InterpMethod _coeff_interp_method;

  /// Closure parameters
  const Real _mu_1_squared = 3. / 7. - 2. / 7. * std::sqrt(6. / 5.);
  const Real _mu_2_squared = 3. / 7. + 2. / 7. * std::sqrt(6. / 5.);
};
