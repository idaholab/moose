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

#ifndef USERFORCINGFUNCTION_H
#define USERFORCINGFUNCTION_H

#include "Kernel.h"

//Forward Declarations
class UserForcingFunction;
class Function;

template<>
InputParameters validParams<UserForcingFunction>();

/**
 * Define the Kernel for a user defined forcing function that looks like:
 *
 * test function * forcing function
 */
class UserForcingFunction : public Kernel
{
public:

  UserForcingFunction(const std::string & name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
  /**
   * Evaluate f at the current quadrature point.
   */
  Real f();

  /**
   * Computes test function * forcing function.
   */
  virtual Real computeQpResidual();

private:
  Function & _func;
};
#endif //USERFORCINGFUNCTION_H
