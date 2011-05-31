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
  // virtual Real computeQpJacobian(); // on-diagonal Jacobian is zero
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _rho_var;
  VariableValue & _rho;

  Real _acceleration;
};
 
#endif
