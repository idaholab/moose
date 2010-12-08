#ifndef GRAVITYFORCE_H
#define GRAVITYFORCE_H

#include "Kernel.h"
#include "Material.h"


//Forward Declarations
class GravityForce;

template<>
InputParameters validParams<GravityForce>();

class GravityForce : public Kernel
{
public:

  GravityForce(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _p_var;
  VariableValue & _p;

  Real _acceleration;
};
 
#endif
