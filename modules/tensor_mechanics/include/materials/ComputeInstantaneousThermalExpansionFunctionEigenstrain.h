/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEINSTANTANEOUSTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H
#define COMPUTEINSTANTANEOUSTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H

#include "ComputeThermalExpansionEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

class ComputeInstantaneousThermalExpansionFunctionEigenstrain;

template <>
InputParameters validParams<ComputeInstantaneousThermalExpansionFunctionEigenstrain>();

/**
 * ComputeInstantaneousThermalExpansionFunctionEigenstrain computes an eigenstrain for thermal
 * expansion according to an instantaneous thermal expansion function.
 */
class ComputeInstantaneousThermalExpansionFunctionEigenstrain
    : public ComputeThermalExpansionEigenstrainBase
{
public:
  ComputeInstantaneousThermalExpansionFunctionEigenstrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) override;

  const VariableValue & _temperature_old;
  Function & _thermal_expansion_function;

  /// Stores the thermal strain as a scalar for use in computing an incremental update to this.
  //@{
  MaterialProperty<Real> & _thermal_strain;
  MaterialProperty<Real> & _thermal_strain_old;
  //@}

  /// Indicates whether we are on the first step, avoiding false positives when restarting
  bool & _step_one;
};

#endif // COMPUTEINSTANTANEOUSTHERMALEXPANSIONFUNCTIONEIGENSTRAIN_H
