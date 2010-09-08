#ifndef MMSCONSTANTAUX_H
#define MMSCONSTANTAUX_H

#include "AuxKernel.h"

class MMSConstantAux;

template<>
InputParameters validParams<MMSConstantAux>();

class MMSConstantAux : public AuxKernel
{
public:
  
 MMSConstantAux(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~MMSConstantAux() {}
  
protected:
  virtual Real computeValue();
};

#endif //MMSCONSTANTAUX_H
