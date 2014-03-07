#ifndef NSGRAVITYPOWER_H
#define NSGRAVITYPOWER_H

#include "Kernel.h"

//Forward Declarations
class NSGravityPower;

template<>
InputParameters validParams<NSGravityPower>();

class NSGravityPower : public Kernel
{
public:

  NSGravityPower(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _momentum_var;
  VariableValue & _momentum;

  Real _acceleration;
};

#endif
