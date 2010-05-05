#ifndef IMPLICITEULER
#define IMPLICITEULER

#include "Kernel.h"

// Forward Declaration
class ImplicitEuler;

template<>
InputParameters validParams<ImplicitEuler>();

class ImplicitEuler : public Kernel
{
public:

  ImplicitEuler(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  

  virtual Real computeQpJacobian();

};
#endif //IMPLICITEULER
