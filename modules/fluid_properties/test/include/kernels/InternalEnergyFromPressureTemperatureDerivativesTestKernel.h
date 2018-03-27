//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERNALENERGYFROMPRESSURETEMPERATUREDERIVATIVESTESTKERNEL_H
#define INTERNALENERGYFROMPRESSURETEMPERATUREDERIVATIVESTESTKERNEL_H

#include "FluidPropertyDerivativesTestKernel.h"

class InternalEnergyFromPressureTemperatureDerivativesTestKernel;

template <>
InputParameters validParams<InternalEnergyFromPressureTemperatureDerivativesTestKernel>();

/**
 * Tests derivatives of internal energy from pressure and temperature.
 */
class InternalEnergyFromPressureTemperatureDerivativesTestKernel
  : public FluidPropertyDerivativesTestKernel
{
public:
  InternalEnergyFromPressureTemperatureDerivativesTestKernel(const InputParameters & parameters);
  virtual ~InternalEnergyFromPressureTemperatureDerivativesTestKernel();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const VariableValue & _p;
  const VariableValue & _T;

  const unsigned int _p_index;
  const unsigned int _T_index;
};

#endif /* INTERNALENERGYFROMPRESSURETEMPERATUREDERIVATIVESTESTKERNEL_H */
