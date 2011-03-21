#ifndef MMSIMPLICITEULER_H_
#define MMSIMPLICITEULER_H_

#include "TimeKernel.h"

class MMSImplicitEuler;

template<>
InputParameters validParams<MMSImplicitEuler>();

class MMSImplicitEuler : public TimeKernel
{
public:
  MMSImplicitEuler(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif //MMSIMPLICITEULER_H_
