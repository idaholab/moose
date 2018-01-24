#ifndef CONSERVATIVEADVECTION_H
#define CONSERVATIVEADVECTION_H

#include "Kernel.h"

// Forward Declaration
class ConservativeAdvection;

template <>
InputParameters validParams<ConservativeAdvection>();

class ConservativeAdvection : public Kernel
{
public:
  ConservativeAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  RealVectorValue _velocity;
};

#endif // CONSERVATIVEADVECTION_H
