#ifndef GRAVITYPOWER_H
#define GRAVITYPOWER_H

#include "Kernel.h"
#include "Material.h"

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

  unsigned int _pv_var;
  VariableValue & _pv;

  Real _acceleration;
};
 
#endif
