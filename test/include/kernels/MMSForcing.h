#ifndef MMSFORCING_H
#define MMSFORCING_H

#include "Kernel.h"

class MMSForcing;

template<>
InputParameters validParams<MMSForcing>();

class MMSForcing : public Kernel
{
public:

  MMSForcing(std::string name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
 
  Real f();
  
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

private:
  
  Real _x;
  Real _y;
  Real _z;
  
};
#endif //MMSFORCING_H
