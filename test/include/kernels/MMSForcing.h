#ifndef MMSFORCING_H_
#define MMSFORCING_H_

#include "Kernel.h"

class MMSForcing;

template<>
InputParameters validParams<MMSForcing>();

class MMSForcing : public Kernel
{
public:
  MMSForcing(const std::string & name, InputParameters parameters);

protected:
  Real f();
  
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  Real _x;
  Real _y;
  Real _z;
  
};

#endif //MMSFORCING_H_
