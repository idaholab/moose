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
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) = 0;

  const VariableValue & _temperature;
  MaterialProperty<RankTwoTensor> & _deigenstrain_dT;
  Real _stress_free_temperature;
};

#endif // COMPUTETHERMALEXPANSIONEIGENSTRAINBASE_H
