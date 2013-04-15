#ifndef VECTORMAGNITUDEAUX_H
#define VECTORMAGNITUDEAUX_H

#include "AuxKernel.h"

class VectorMagnitudeAux;

template<>
InputParameters validParams<VectorMagnitudeAux>();

/**
 *
 */
class VectorMagnitudeAux : public AuxKernel
{
public:
  VectorMagnitudeAux(const std::string & name, InputParameters parameters);
  virtual ~VectorMagnitudeAux();

protected:
  virtual Real computeValue();

  VariableValue & _x;
  VariableValue & _y;
  VariableValue & _z;
};

#endif /* VECTORMAGNITUDEAUX_H */
