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

  GravityForce(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _p_var;
  MooseArray<Real> & _p;

  Real _acceleration;
};
 
#endif
