//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ENTROPYFROMENTHALPYPRESSUREDERIVATIVESTESTKERNEL_H
#define ENTROPYFROMENTHALPYPRESSUREDERIVATIVESTESTKERNEL_H

#include "FluidPropertyDerivativesTestKernel.h"

class EntropyFromEnthalpyPressureDerivativesTestKernel;

template <>
InputParameters validParams<EntropyFromEnthalpyPressureDerivativesTestKernel>();

/**
 * Tests derivatives of specific entropy from specific enthalpy and pressure
 */
class EntropyFromEnthalpyPressureDerivativesTestKernel : public FluidPropertyDerivativesTestKernel
{
public:
  EntropyFromEnthalpyPressureDerivativesTestKernel(const InputParameters & parameters);
  virtual ~EntropyFromEnthalpyPressureDerivativesTestKernel();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// specific enthalpy
  const VariableValue & _h;
  /// pressure
  const VariableValue & _p;

  /// specific enthalpy coupled variable index
  const unsigned int _h_index;
  /// pressure coupled variable index
  const unsigned int _p_index;
};

#endif /* ENTROPYFROMENTHALPYPRESSUREDERIVATIVESTESTKERNEL_H */
