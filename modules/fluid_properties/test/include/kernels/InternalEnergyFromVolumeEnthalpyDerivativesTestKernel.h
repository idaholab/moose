//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERNALENERGYFROMVOLUMEENTHALPYDERIVATIVESTESTKERNEL_H
#define INTERNALENERGYFROMVOLUMEENTHALPYDERIVATIVESTESTKERNEL_H

#include "FluidPropertyDerivativesTestKernel.h"

class InternalEnergyFromVolumeEnthalpyDerivativesTestKernel;

template <>
InputParameters validParams<InternalEnergyFromVolumeEnthalpyDerivativesTestKernel>();

/**
 * Tests derivatives of specific internal energy from specific volume and specific enthalpy
 */
class InternalEnergyFromVolumeEnthalpyDerivativesTestKernel
  : public FluidPropertyDerivativesTestKernel
{
public:
  InternalEnergyFromVolumeEnthalpyDerivativesTestKernel(const InputParameters & parameters);
  virtual ~InternalEnergyFromVolumeEnthalpyDerivativesTestKernel();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// specific volume
  const VariableValue & _v;
  /// specific enthalpy
  const VariableValue & _h;

  /// specific volume coupled variable index
  const unsigned int _v_index;
  /// specific enthalpy coupled variable index
  const unsigned int _h_index;
};

#endif /* INTERNALENERGYFROMVOLUMEENTHALPYDERIVATIVESTESTKERNEL_H */
