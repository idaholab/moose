/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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

  JouleHeating(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  VariableGradient& _grad_potential;
  MaterialProperty<Real> & _thermal_conductivity;
};

#endif
