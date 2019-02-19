#ifndef VOLUMEJUNCTIONPRESSUREVUAUX_H
#define VOLUMEJUNCTIONPRESSUREVUAUX_H

#include "AuxScalarKernel.h"

class VolumeJunctionPressureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<VolumeJunctionPressureAux>();

/**
 * Computes pressure for the volume junction component
 */
class VolumeJunctionPressureAux : public AuxScalarKernel
{
public:
  VolumeJunctionPressureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _rhoe;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* VOLUMEJUNCTIONPRESSUREVUAUX_H */
