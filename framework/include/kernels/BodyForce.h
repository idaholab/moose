#ifndef BODYFORCE_H
#define BODYFORCE_H

#include "Kernel.h"

//Forward Declarations
class BodyForce;
class Function;

template<>
InputParameters validParams<BodyForce>();

class BodyForce : public Kernel
{
public:

  BodyForce(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  Real _value;
  const bool _has_function;
  Function * const _function;
};
 
#endif
