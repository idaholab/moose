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

#ifndef CONVECTION_H
#define CONVECTION_H

#include "Kernel.h"

class Convection;

template<>
InputParameters validParams<Convection>();

class Convection : public Kernel
{
public:

  Convection(const std::string & name,
             InputParameters parameters);

protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  
  VariableGradient & _some_variable;
};

#endif //CONVECTION_H
