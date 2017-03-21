/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEMEANTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H
#define COMPUTEMEANTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H

#include "ComputeThermalExpansionEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

class ComputeMeanThermalExpansionFunctionEigenstrain;

template <>
InputParameters validParams<ComputeMeanThermalExpansionFunctionEigenstrain>();

/**
 * ComputeMeanThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion according to a mean thermal expansion function.
 */
class ComputeMeanThermalExpansionFunctionEigenstrain : public ComputeThermalExpansionEigenstrainBase
{
public:
  ComputeMeanThermalExpansionFunctionEigenstrain(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) override;

  Function & _thermal_expansion_function;
  const Real & _reference_temperature;
  Real _alphabar_stress_free_temperature;
  Real _thexp_stress_free_temperature;
};

#endif // COMPUTEMEANTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H
