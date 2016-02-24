/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef JOULEHEATING_H
#define JOULEHEATING_H

#include "Kernel.h"

//Forward Declarations
class JouleHeating;
class Function;

template<>
InputParameters validParams<JouleHeating>();

class JouleHeating : public Kernel
{
public:

  JouleHeating(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const VariableGradient & _grad_potential;
  const MaterialProperty<Real> & _thermal_conductivity;
};

#endif
