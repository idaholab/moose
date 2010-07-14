#ifndef POLYFORCING_H
#define POLYFORCING_H

#include "Kernel.h"

class PolyForcing;

template<>
InputParameters validParams<PolyForcing>();

class PolyForcing : public Kernel
{
public:

  PolyForcing(std::string name,
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
#endif //POLYFORCING_H
