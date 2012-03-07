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

#ifndef EXPLICITODE_H_
#define EXPLICITODE_H_

#include "AuxScalarKernel.h"

//Forward Declarations
class ExplicitODE;

template<>
InputParameters validParams<ExplicitODE>();

/**
 * Explicit solve of ODE:
 *
 * dy/dt = -\lambda y  (using forward Euler)
 */
class ExplicitODE : public AuxScalarKernel
{
public:
  ExplicitODE(const std::string & name, InputParameters parameters);
  virtual ~ExplicitODE();

protected:
  virtual Real computeValue();

  Real _lambda;
};

#endif /* EXPLICITODE_H_ */
