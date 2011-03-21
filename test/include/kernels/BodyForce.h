#ifndef BODYFORCE_H_
#define BODYFORCE_H_

#include "Kernel.h"

//Forward Declarations
class BodyForce;

template<>
InputParameters validParams<BodyForce>();

class BodyForce : public Kernel
{
public:

  BodyForce(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  Real _value;
};
 
#endif
