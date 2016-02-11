#ifndef DGADVECTION_H
#define DGADVECTION_H

#include "DGKernel.h"

class DGAdvection;

template<>
InputParameters validParams<DGAdvection>();

class DGAdvection : public DGKernel
{
public:
  DGAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  RealVectorValue _velocity;
};

#endif //DGADVECTION_H
