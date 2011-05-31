#ifndef GRAVITYPOWER_H
#define GRAVITYPOWER_H

#include "Kernel.h"

//Forward Declarations
class GravityPower;

template<>
InputParameters validParams<GravityPower>();

class GravityPower : public Kernel
{
public:

  GravityPower(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _momentum_var;
  VariableValue & _momentum;

  Real _acceleration;
};
 
#endif
