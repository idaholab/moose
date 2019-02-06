#ifndef HYDRAULICDIAMETERAUX_H
#define HYDRAULICDIAMETERAUX_H

#include "AuxKernel.h"

class HydraulicDiameterCircularAux;

template <>
InputParameters validParams<HydraulicDiameterCircularAux>();

/**
 * Computes hydraulic diameter for a circular pipe
 */
class HydraulicDiameterCircularAux : public AuxKernel
{
public:
  HydraulicDiameterCircularAux(const InputParameters & parameters);

protected:
  Real computeValue();

  const VariableValue & _area;
};

#endif /* HYDRAULICDIAMETERAUX_H */
