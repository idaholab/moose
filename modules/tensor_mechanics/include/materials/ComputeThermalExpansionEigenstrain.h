/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTETHERMALEXPANSIONEIGENSTRAIN_H
#define COMPUTETHERMALEXPANSIONEIGENSTRAIN_H

#include "ComputeEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

class ComputeThermalExpansionEigenstrain;
class RankTwoTensor;

template<>
InputParameters validParams<ComputeThermalExpansionEigenstrain>();

/**
 * ComputeThermalExpansionEigenstrain computes an Eigenstrain corresponding to the
 * thermal expansion of a material.
 */
class ComputeThermalExpansionEigenstrain : public DerivativeMaterialInterface<ComputeEigenstrainBase>
{
public:
  ComputeThermalExpansionEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  const VariableValue & _temperature;
  MaterialProperty<RankTwoTensor> & _deigenstrain_dT;
  const Real & _thermal_expansion_coeff;
  Real _stress_free_temperature;
};

#endif // COMPUTETHERMALEXPANSIONEIGENSTRAIN_H
