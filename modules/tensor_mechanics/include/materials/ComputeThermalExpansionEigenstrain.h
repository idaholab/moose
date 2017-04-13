/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTETHERMALEXPANSIONEIGENSTRAIN_H
#define COMPUTETHERMALEXPANSIONEIGENSTRAIN_H

#include "ComputeThermalExpansionEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

class ComputeThermalExpansionEigenstrain;

template <>
InputParameters validParams<ComputeThermalExpansionEigenstrain>();

/**
 * ComputeThermalExpansionEigenstrain computes an eigenstrain for thermal expansion
 * with a constant expansion coefficient.
 */
class ComputeThermalExpansionEigenstrain : public ComputeThermalExpansionEigenstrainBase
{
public:
  ComputeThermalExpansionEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) override;

  const Real & _thermal_expansion_coeff;
};

#endif // COMPUTETHERMALEXPANSIONEIGENSTRAIN_H
