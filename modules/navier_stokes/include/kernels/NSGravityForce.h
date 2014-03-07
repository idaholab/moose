#ifndef NSGRAVITYFORCE_H
#define NSGRAVITYFORCE_H

#include "NSKernel.h"


// Forward Declarations
class NSGravityForce;

template<>
InputParameters validParams<NSGravityForce>();

class NSGravityForce : public NSKernel
{
public:

  NSGravityForce(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  Real _acceleration;
};

#endif
