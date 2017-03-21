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

#ifndef TIMEDERIVATIVENODALKERNEL_H
#define TIMEDERIVATIVENODALKERNEL_H

#include "TimeNodalKernel.h"

// Forward Declarations
class TimeDerivativeNodalKernel;

template <>
InputParameters validParams<TimeDerivativeNodalKernel>();

/**
 * Represents du/dt
 */
class TimeDerivativeNodalKernel : public TimeNodalKernel
{
public:
  /**
   * Constructor (Comment here for @aeslaughter :-)
   */
  TimeDerivativeNodalKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;
};

#endif
