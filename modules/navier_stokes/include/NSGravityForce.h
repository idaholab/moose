#ifndef NSGRAVITYFORCE_H
#define NSGRAVITYFORCE_H

#include "Kernel.h"
#include "Material.h"


//Forward Declarations
class NSGravityForce;

template<>
InputParameters validParams<NSGravityForce>();

class NSGravityForce : public Kernel
{
public:

  NSGravityForce(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  // virtual Real computeQpJacobian(); // on-diagonal Jacobian is zero
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _rho_var;
  VariableValue & _rho;

  Real _acceleration;
};
 
#endif
