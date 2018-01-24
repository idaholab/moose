#ifndef PRESSUREAUX_H
#define PRESSUREAUX_H

#include "AuxKernel.h"

class PressureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<PressureAux>();

/**
 * Compute temperature values from specific volume and internal energy
 */
class PressureAux : public AuxKernel
{
public:
  PressureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _v;
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* PRESSUREAUX_H */
