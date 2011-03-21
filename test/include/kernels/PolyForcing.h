#ifndef POLYFORCING_H_
#define POLYFORCING_H_

#include "Kernel.h"

class PolyForcing;

template<>
InputParameters validParams<PolyForcing>();

class PolyForcing : public Kernel
{
public:
  PolyForcing(const std::string & name, InputParameters parameters);

protected:
  Real f();
  
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _x;
  Real _y;
  Real _z;
  
};

#endif //POLYFORCING_H
