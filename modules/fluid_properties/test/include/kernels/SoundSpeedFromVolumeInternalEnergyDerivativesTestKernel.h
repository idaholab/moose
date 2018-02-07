//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SOUNDSPEEDFROMVOLUMEINTERNALENERGYDERIVATIVESTESTKERNEL_H
#define SOUNDSPEEDFROMVOLUMEINTERNALENERGYDERIVATIVESTESTKERNEL_H

#include "FluidPropertyDerivativesTestKernel.h"

class SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel;

template <>
InputParameters validParams<SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel>();

/**
 * Tests derivatives of sound speed from specific volume and specific internal energy
 */
class SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel
  : public FluidPropertyDerivativesTestKernel
{
public:
  SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel(const InputParameters & parameters);
  virtual ~SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// specific volume
  const VariableValue & _v;
  /// specific internal energy
  const VariableValue & _e;

  /// specific volume coupled variable index
  const unsigned int _v_index;
  /// specific internal energy coupled variable index
  const unsigned int _e_index;
};

#endif /* SOUNDSPEEDFROMVOLUMEINTERNALENERGYDERIVATIVESTESTKERNEL_H */
