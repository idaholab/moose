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

// Forward Declarations
class ConstantRate;

template <>
InputParameters validParams<ConstantRate>();

/**
 * Represents the rate in a simple ODE of du/dt = rate
 */
class ConstantRate : public NodalKernel
{
public:
  /**
   * Constructor initializes the rate
   */
  ConstantRate(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  /// The rate
  Real _rate;
};

#endif
