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

/// FVSP3TemperatureSourceSink implements irradiation-consistent temperature source
///
///     - strong form: \int dV \int d\nu \nabla \cdot (1/\kappa) \nabla (a1 * psi_1 + a2 * psi_2)
///
///     - Integrated by parts : \int_{A} dA \int d\nu (1/\kappa)_f \nabla (a1 * psi_1 + a2 * psi_2)_f
///
class FVSP3TemperatureSourceSink : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVSP3TemperatureSourceSink(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// kappa absorptivities per band
  const std::vector<MooseFunctorName> & _absorptivity_vec;

  /// Order 1 thermal radiation flux moments
  const std::vector<MooseFunctorName> & _psi1_vec;

  /// Order 2 thermal radiation flux moments
  const std::vector<MooseFunctorName> & _psi2_vec;

  /// Vector to store the functors
  std::vector<const Moose::Functor<ADReal> *> _absorptivity_vec_functors;
  std::vector<const Moose::Functor<ADReal> *> _psi1_vec_functors;
  std::vector<const Moose::Functor<ADReal> *> _psi2_vec_functors;

  /// Closure parameters
  const Real _a1 = 1. / 30. * (5. - 3. * std::sqrt(5. / 6.));
  const Real _a2 = 1. / 30. * (5. + 3. * std::sqrt(5. / 6.));
};
