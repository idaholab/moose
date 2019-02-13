#ifndef DUMMYAUX_H
#define DUMMYAUX_H

#include "AuxKernel.h"

class DummyAux;

template <>
InputParameters validParams<DummyAux>();

class DummyAux : public AuxKernel
{
public:
  DummyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
};

#endif // CHECKCURRENTEXECAUX_H
