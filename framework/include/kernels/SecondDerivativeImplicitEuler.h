#ifndef SECONDDERIVATIVEIMPLICITEULER_H
#define SECONDDERIVATIVEIMPLICITEULER_H

#include "Kernel.h"

//Forward Declarations
class SecondDerivativeImplicitEuler;

template<>
InputParameters validParams<SecondDerivativeImplicitEuler>();

class SecondDerivativeImplicitEuler : public Kernel
{
public:

  SecondDerivativeImplicitEuler(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

};
#endif //SECONDDERIVATIVEIMPLICITEULER_H
