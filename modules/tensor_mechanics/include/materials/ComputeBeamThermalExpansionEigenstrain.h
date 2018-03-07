//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEBEAMTHERMALEXPANSIONEIGENSTRAIN_H
#define COMPUTEBEAMTHERMALEXPANSIONEIGENSTRAIN_H

#include "ComputeBeamThermalExpansionEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

class ComputeBeamThermalExpansionEigenstrain;

template <>
InputParameters validParams<ComputeBeamThermalExpansionEigenstrain>();

/**
 * ComputeBeamThermalExpansionEigenstrain computes an eigenstrain for thermal expansion
 * with a constant expansion coefficient.
 */
class ComputeBeamThermalExpansionEigenstrain : public ComputeBeamThermalExpansionEigenstrainBase
{
public:
  ComputeBeamThermalExpansionEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeThermalStrain(Real & thermal_strain, Real & instantaneous_cte) override;

  /// Constant thermal expansion coefficient
  const Real & _thermal_expansion_coeff;
};

#endif // COMPUTEBEAMTHERMALEXPANSIONEIGENSTRAIN_H
