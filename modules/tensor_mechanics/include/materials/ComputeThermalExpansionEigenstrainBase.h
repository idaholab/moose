//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTETHERMALEXPANSIONEIGENSTRAINBASE_H
#define COMPUTETHERMALEXPANSIONEIGENSTRAINBASE_H

#include "ComputeEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

class ComputeThermalExpansionEigenstrainBase;
class RankTwoTensor;

template <>
InputParameters validParams<ComputeThermalExpansionEigenstrainBase>();

/**
 * ComputeThermalExpansionEigenstrainBase is a base class for all models that
 * compute eigenstrains due to thermal expansion of a material.
 */
class ComputeThermalExpansionEigenstrainBase
    : public DerivativeMaterialInterface<ComputeEigenstrainBase>
{
public:
  ComputeThermalExpansionEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;
  /*
   * Compute the total thermal strain relative to the stress-free temperature at
   * the current temperature, as well as the current instantaneous thermal
   * expansion coefficient.
   * param thermal_strain    The current total linear thermal strain
   *                         (\delta L / L)
   * param instantaneous_cte The current instantaneous coefficient of thermal
   *                         expansion (derivative of thermal_strain wrt
   *                         temperature
   */
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) = 0;

  const VariableValue & _temperature;
  MaterialProperty<RankTwoTensor> & _deigenstrain_dT;
  const VariableValue & _stress_free_temperature;
};

#endif // COMPUTETHERMALEXPANSIONEIGENSTRAINBASE_H
