#ifndef BODYFORCE_H
#define BODYFORCE_H

#include "Kernel.h"

//Forward Declarations
class BodyForce;

template<>
InputParameters validParams<BodyForce>();

class BodyForce : public Kernel
{
public:

  BodyForce(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

private:
  Real _value;
};
 
#endif
