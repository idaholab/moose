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
#ifndef MMSIMPLICITEULER_H_
#define MMSIMPLICITEULER_H_

#include "TimeKernel.h"

class MMSImplicitEuler;

template <>
InputParameters validParams<MMSImplicitEuler>();

class MMSImplicitEuler : public TimeKernel
{
public:
  MMSImplicitEuler(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableValue & _u_old;
};

#endif // MMSIMPLICITEULER_H_
