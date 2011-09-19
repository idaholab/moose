#ifndef POLYCONSTANTAUX_H
#define POLYCONSTANTAUX_H

#include "AuxKernel.h"

class PolyConstantAux;

template<>
InputParameters validParams<PolyConstantAux>();

class PolyConstantAux : public AuxKernel
{
public:
  PolyConstantAux(const std::string & name, InputParameters parameters);

  virtual ~PolyConstantAux() {}

protected:
  virtual Real computeValue();
};

#endif //POLYCONSTANTAUX_H
