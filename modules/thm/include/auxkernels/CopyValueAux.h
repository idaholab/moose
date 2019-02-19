#ifndef COPYVALUEAUX_H
#define COPYVALUEAUX_H

#include "AuxKernel.h"

class CopyValueAux;

template <>
InputParameters validParams<CopyValueAux>();

/**
 *
 */
class CopyValueAux : public AuxKernel
{
public:
  CopyValueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _source_var;
};

#endif /* COPYVALUEAUX_H */
