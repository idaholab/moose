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

#ifndef TIMENODALKERNEL_H
#define TIMENODALKERNEL_H

#include "NodalKernel.h"

//Forward Declarations
class TimeNodalKernel;
class Function;

template<>
InputParameters validParams<TimeNodalKernel>();

/**
 * Represents a simple ODE of du/dt - rate = 0
 */
class TimeNodalKernel : public NodalKernel
{
public:
  /**
   * Constructor initializes the rate
   */
  TimeNodalKernel(const InputParameters & parameters);

protected:

  /**
   * Adds to the "time residual" for any NodalKernel inheriting
   * from this class
   */
  virtual void computeResidual();
};

#endif
