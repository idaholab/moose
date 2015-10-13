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

#ifndef CONSTANTRATE_H
#define CONSTANTRATE_H

#include "NodalKernel.h"

//Forward Declarations
class ConstantRate;
class Function;

template<>
InputParameters validParams<ConstantRate>();

/**
 * Represents a simple ODE of du/dt - rate = 0
 */
class ConstantRate : public NodalKernel
{
public:
  /**
   * Constructor initializes the rate
   */
  ConstantRate(const InputParameters & parameters);

protected:
  /**
   * Implement du/dt - rate
   */
  virtual Real computeQpResidual();

  /// The rate
  Real _rate;
};

#endif
