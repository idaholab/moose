#ifndef HYDRAULICDIAMETERAUX_H
#define HYDRAULICDIAMETERAUX_H

#include "AuxKernel.h"

class HydraulicDiameterAux;

template<>
InputParameters validParams<HydraulicDiameterAux>();

/**
 * Computes hydraulic diameter for a circular pipe
 */
class HydraulicDiameterCircularAux : public AuxKernel
{
public:
  HydraulicDiameterCircularAux(const InputParameters & parameters);
  virtual ~HydraulicDiameterCircularAux();

protected:
  Real computeValue();

  VariableValue & _area;
};


#endif /* HYDRAULICDIAMETERAUX_H */
