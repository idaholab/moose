#ifndef THMSPECIFICVOLUMEAUX_H
#define THMSPECIFICVOLUMEAUX_H

#include "AuxKernel.h"

class THMSpecificVolumeAux;

template <>
InputParameters validParams<THMSpecificVolumeAux>();

/**
 * Computes specific volume
 */
class THMSpecificVolumeAux : public AuxKernel
{
public:
  THMSpecificVolumeAux(const InputParameters & parameters);

protected:
  Real computeValue();

  const VariableValue & _rhoA;
  const VariableValue & _area;
  const VariableValue & _alpha;
};

#endif /* THMSPECIFICVOLUMEAUX_H */
