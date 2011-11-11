#ifndef CONVECTION_H
#define CONVECTION_H

#include "Kernel.h"

// Forward Declaration
class Convection;


template<>
InputParameters validParams<Convection>();

class Convection : public Kernel
{
public:

  Convection(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  RealVectorValue _velocity;
};

#endif // CONVECTION_H
