#ifndef CONVECTION_H
#define CONVECTION_H

#include "Kernel.h"

class Convection;

template<>
InputParameters validParams<Convection>();

class Convection : public Kernel
{
public:

  Convection(std::string name,
             MooseSystem &sys,
             InputParameters parameters);

protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  
  VariableGradient & _velocity_vector;
};

#endif //CONVECTION_H
