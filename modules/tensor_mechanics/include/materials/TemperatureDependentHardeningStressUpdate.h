//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IsotropicPlasticityStressUpdate.h"
#include "LinearInterpolation.h"

class PiecewiseLinear;

/**
 * This class inherits from IsotropicPlasticityStressUpdate. It
 * calculates stress as a function of temperature and plastic strain by
 * interpolating hardening functions at different temperatures input by the user.
 */
class TemperatureDependentHardeningStressUpdate : public IsotropicPlasticityStressUpdate
{
public:
  static InputParameters validParams();

  TemperatureDependentHardeningStressUpdate(const InputParameters & parameters);

protected:
  virtual void computeStressInitialize(const Real & effectiveTrialStress,
                                       const RankFourTensor & elasticity_tensor) override;

  virtual void computeYieldStress(const RankFourTensor & elasticity_tensor) override;
  virtual Real computeHardeningValue(Real scalar) override;
  virtual Real computeHardeningDerivative(Real scalar) override;

  /**
   * Determines the section of the piecewise temperature dependent hardening
   * function for the current temperature and calculates the relative temperature
   * fraction within that piecewise function section.
   */
  void initializeHardeningFunctions();

  MooseSharedPointer<LinearInterpolation> _interp_yield_stress;

  ///@{The function names and expressions for hardening as a function of temperature
  const std::vector<FunctionName> _hardening_functions_names;
  std::vector<const PiecewiseLinear *> _hardening_functions;
  ///@}

  /// The temperatures at which each of the hardening functions are defined.
  std::vector<Real> _hf_temperatures;

  ///@{Indices to identify the lower and upper temperature bounds for the current value
  unsigned int _hf_index_lo;
  unsigned int _hf_index_hi;
  ///@}

  /**
   * The fraction of the temperature within the bounds of the relevant section
   * of the piecewise hardening function. This hardening temperature fraction is
   * used in the interpolation of the hardening value, which is a piecewise
   * function of the temperature.
   */
  Real _hf_fraction;
};
