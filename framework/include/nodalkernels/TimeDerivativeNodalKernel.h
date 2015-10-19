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

#include "NodalKernel.h"

//Forward Declarations
class TimeDerivativeNodalKernel;
class Function;

template<>
InputParameters validParams<TimeDerivativeNodalKernel>();

/**
 * Represents du/dt
 */
class TimeDerivativeNodalKernel : public NodalKernel
{
public:
  /**
   * Constructor (Comment here for @aeslaughter :-)
   */
  TimeDerivativeNodalKernel(const InputParameters & parameters);

protected:
  /**
   * Implement du/dt
   */
  virtual Real computeQpResidual();

  /**
   * Jacobian with respect to the variable this NodalKernel is operating on.
   */
  virtual Real computeQpJacobian();
};

#endif
