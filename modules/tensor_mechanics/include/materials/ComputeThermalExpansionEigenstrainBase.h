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
  virtual ValueAndDerivative<is_ad> computeThermalStrain() = 0;

  /**
   * temperature to use in the eigenstrain calculation (depending on _use_old_temperature).
   * We use a const reference to a private member here to prevent derived classes for
   * accidentally overwriting any values.
   */
  const std::vector<ValueAndDerivative<is_ad>> & _temperature;

  /// lag temperature variable
  const bool _use_old_temperature;

  /// previous time step temperature
  const VariableValue & _temperature_old;

  MaterialProperty<RankTwoTensor> * _deigenstrain_dT;

  const VariableValue & _stress_free_temperature;

  using ComputeEigenstrainBaseTempl<is_ad>::_qp;
  using ComputeEigenstrainBaseTempl<is_ad>::_eigenstrain;
  using ComputeEigenstrainBaseTempl<is_ad>::_eigenstrain_name;

private:
  /// temperature to use in the eigenstrain calculation (depending on _use_old_temperature)
  std::vector<ValueAndDerivative<is_ad>> _temperature_buffer;

  /// current temperature
  const GenericVariableValue<is_ad> & _temperature_prop;
};

typedef ComputeThermalExpansionEigenstrainBaseTempl<false> ComputeThermalExpansionEigenstrainBase;
typedef ComputeThermalExpansionEigenstrainBaseTempl<true> ADComputeThermalExpansionEigenstrainBase;
