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
#include "DerivativeMaterialInterface.h"

/**
 * ADComputeInstantaneousThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion according to an instantaneous thermal expansion function.
 */
class ADComputeInstantaneousThermalExpansionFunctionEigenstrain
  : public ADComputeThermalExpansionEigenstrainBase
{
public:
  static InputParameters validParams();

  ADComputeInstantaneousThermalExpansionFunctionEigenstrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeThermalStrain(ADReal & thermal_strain) override;

  const VariableValue & _temperature_old;
  const Function & _thermal_expansion_function;

  /// Stores the thermal strain as a scalar for use in computing an incremental update to this.
  //@{
  ADMaterialProperty<Real> & _thermal_strain;
  const MaterialProperty<Real> & _thermal_strain_old;
  //@}

  /// Indicates whether we are on the first step, avoiding false positives when restarting
  bool & _step_one;

  using ADComputeThermalExpansionEigenstrainBase::computeThermalStrain;
};
