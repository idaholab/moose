/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
