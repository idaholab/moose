/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTETHERMALEXPANSIONEIGENSTRAIN_H
#define COMPUTETHERMALEXPANSIONEIGENSTRAIN_H

#include "ComputeStressFreeStrainBase.h"

class ComputeThermalExpansionEigenStrain;

template<>
InputParameters validParams<ComputeThermalExpansionEigenStrain>();

/**
 * ComputeThermalExpansionEigenStrain computes an Eigenstrain corresponding to the
 * thermal expansion of a material.
 */
class ComputeThermalExpansionEigenStrain : public ComputeStressFreeStrainBase
{
public:
  ComputeThermalExpansionEigenStrain(const InputParameters & parameters);

protected:
  virtual void computeQpStressFreeStrain();

  const VariableValue & _temperature;
  bool _has_incremental_strain;
  const VariableValue * _temperature_old;
  const Real & _thermal_expansion_coeff;
  const Real & _stress_free_reference_temperature;
};

#endif // COMPUTETHERMALEXPANSIONEIGENSTRAIN_H
