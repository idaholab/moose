#ifndef MMSIMPLICITEULER
#define MMSIMPLICITEULER

#include "Kernel.h"

class MMSImplicitEuler;

template<>
InputParameters validParams<MMSImplicitEuler>();

class MMSImplicitEuler : public Kernel
{
public:

  MMSImplicitEuler(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  

  virtual Real computeQpJacobian();

};
#endif //MMSIMPLICITEULER
