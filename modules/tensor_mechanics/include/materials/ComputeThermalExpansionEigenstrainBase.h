//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensorForward.h"
#include "ChainedReal.h"

/// Return type with a single derivative
template <bool is_ad>
using ValueAndDerivative = typename std::conditional<is_ad, ADReal, ChainedReal>::type;

/**
 * ComputeThermalExpansionEigenstrainBase is a base class for all models that
 * compute eigenstrains due to thermal expansion of a material.
 */
template <bool is_ad>
class ComputeThermalExpansionEigenstrainBaseTempl
  : public DerivativeMaterialInterface<ComputeEigenstrainBaseTempl<is_ad>>
{
public:
  static InputParameters validParams();

  ComputeThermalExpansionEigenstrainBaseTempl(const InputParameters & parameters);

  /// resize _temperature_buffer
  virtual void subdomainSetup() final;

  /// update _temperature_buffer
  virtual void computeProperties() final;

  virtual void computeQpEigenstrain() override;

protected:
  /**
   * computeThermalStrain must be overridden in derived classes. The return type
   * ValueAndDerivative<is_ad> contains the value for the thermal strain and its
   * temperature derivative. Derived classes should use `_temperature[_qp]` to obtain
   * the current temperature. In the is_ad == false case that member variable is
   * agumented and will be of the type ChainedReal. I.e. even with is_ad == false
   * a variant of forward mode automatic differentiation will be used internally to
   * compute the thermal strain and no manual implementation of the temperature derivative
   * is needed.
   *
   * @return thermal strain and its derivative, where the thermal strain is the linear thermal
   * strain (\delta L / L)
   */
  virtual ValueAndDerivative<is_ad> computeThermalStrain() = 0;

  /**
   * Temperature to use in the eigenstrain calculation (current value if _use_old_temperature=false,
   * old value if _use_old_temperature=true). We use a const reference to a private member here to
   * prevent derived classes from accidentally overwriting any values.
   */
  const std::vector<ValueAndDerivative<is_ad>> & _temperature;

  /// lag temperature variable
  const bool _use_old_temperature;

  /// previous time step temperature
  const VariableValue & _temperature_old;

  // this temperature derivative property is onlycreated and set for is_ad == false
  MaterialProperty<RankTwoTensor> * _deigenstrain_dT;

  const VariableValue & _stress_free_temperature;

  using ComputeEigenstrainBaseTempl<is_ad>::_qp;
  using ComputeEigenstrainBaseTempl<is_ad>::_eigenstrain;
  using ComputeEigenstrainBaseTempl<is_ad>::_eigenstrain_name;

private:
  /**
   * Temperature used in the eigenstrain calculation (current value if _use_old_temperature=false,
   * old value if _use_old_temperature=true).
   */
  std::vector<ValueAndDerivative<is_ad>> _temperature_buffer;

  /// current temperature
  const GenericVariableValue<is_ad> & _temperature_prop;

  /// mean coefficient of thermal expansion (for output verification)
  MaterialProperty<Real> * _mean_thermal_expansion_coefficient;
};

typedef ComputeThermalExpansionEigenstrainBaseTempl<false> ComputeThermalExpansionEigenstrainBase;
typedef ComputeThermalExpansionEigenstrainBaseTempl<true> ADComputeThermalExpansionEigenstrainBase;
