#ifndef NIPG0_H
#define NIPG0_H

#include "DGKernel.h"

//Forward Declarations
class NIPG0;

template<>
InputParameters validParams<NIPG0>();

class NIPG0 : public DGKernel
{
public:
  NIPG0(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  Real _e;
  Real _s;
};
 
#endif
