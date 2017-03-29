#ifndef SPECIFICVOLUMEAUX_H
#define SPECIFICVOLUMEAUX_H

#include "AuxKernel.h"

class SpecificVolumeAux;

template <>
InputParameters validParams<SpecificVolumeAux>();

/**
 * Computes specific volume
 */
class SpecificVolumeAux : public AuxKernel
{
public:
  SpecificVolumeAux(const InputParameters & parameters);
  virtual ~SpecificVolumeAux();

protected:
  Real computeValue();

  const VariableValue & _rhoA;
  const VariableValue & _area;
  const VariableValue & _alpha;
};

#endif /* SPECIFICVOLUMEAUX_H */
