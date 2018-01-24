//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
