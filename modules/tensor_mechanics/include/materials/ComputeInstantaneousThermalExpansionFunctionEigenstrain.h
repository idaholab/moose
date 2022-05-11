//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeThermalExpansionEigenstrainBase.h"

/**
 * ComputeInstantaneousThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion according to an instantaneous thermal expansion function.
 */
template <bool is_ad>
class ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl
  : public ComputeThermalExpansionEigenstrainBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual ValueAndDerivative<is_ad> computeThermalStrain() override;

  const Function & _thermal_expansion_function;

  /**
   *@{ Stores the thermal strain as a scalar for use in computing an incremental update.
   */
  GenericMaterialProperty<Real, is_ad> & _thermal_strain;
  const MaterialProperty<Real> & _thermal_strain_old;
  //@}

  /// Indicates whether we are on the first step, avoiding false positives when restarting
  bool & _step_one;

  using ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::_qp;
};

typedef ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<false>
    ComputeInstantaneousThermalExpansionFunctionEigenstrain;
typedef ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<true>
    ADComputeInstantaneousThermalExpansionFunctionEigenstrain;
