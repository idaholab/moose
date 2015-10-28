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

#ifndef USERFORCINGFUNCTIONNODALKERNEL_H
#define USERFORCINGFUNCTIONNODALKERNEL_H

#include "NodalKernel.h"

//Forward Declarations
class UserForcingFunctionNodalKernel;
class Function;

template<>
InputParameters validParams<UserForcingFunctionNodalKernel>();

/**
 * Represents the rate in a simple ODE of du/dt = f
 */
class UserForcingFunctionNodalKernel : public NodalKernel
{
public:
  /**
   * Constructor grabs the Function
   */
  UserForcingFunctionNodalKernel(const InputParameters & parameters);

protected:
  /**
   * Implement -f
   */
  virtual Real computeQpResidual();

  Function & _func;
};

#endif
