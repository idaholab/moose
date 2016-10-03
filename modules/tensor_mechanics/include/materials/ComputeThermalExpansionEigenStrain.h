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
class RankTwoTensor;

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
  MaterialProperty <RankTwoTensor> & _thermal_expansion_tensor;
  const Real & _stress_free_reference_temperature;

private:
  /// True if this is the first timestep (timestep < 2). At the first
  /// timestep, the change in temperature should be calculated with the reference
  /// stress free temperature, not the stateful _temperature_old; this boolean variable
  /// eliminates the use of _app.isRestarting() in this class.
  /// This boolean is delcared as a reference so that the variable is restartable
  /// data:  if we restart, the code will not think it is the first timestep again.
  bool & _step_one;
};

#endif // COMPUTETHERMALEXPANSIONEIGENSTRAIN_H
