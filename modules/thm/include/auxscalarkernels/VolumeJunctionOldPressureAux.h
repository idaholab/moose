#pragma once

#include "AuxScalarKernel.h"

class VolumeJunctionOldPressureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<VolumeJunctionOldPressureAux>();

/**
 * Computes pressure for the volume junction component
 */
class VolumeJunctionOldPressureAux : public AuxScalarKernel
{
public:
  VolumeJunctionOldPressureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _rhoe;

  const SinglePhaseFluidProperties & _fp;
};
