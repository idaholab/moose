#ifndef TEMPERATUREAUX_H
#define TEMPERATUREAUX_H

#include "AuxKernel.h"

class TemperatureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<TemperatureAux>();

/**
 * Compute temperature values from specific volume and internal energy
 */
class TemperatureAux : public AuxKernel
{
public:
  TemperatureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _v;
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* TEMPERATUREAUX_H */
