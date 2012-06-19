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

#ifndef USEROBJECTKERNEL_H
#define USEROBJECTKERNEL_H

#include "Kernel.h"
#include "MTUserObject.h"

class UserObjectKernel;

template<>
InputParameters validParams<UserObjectKernel>();

/**
 * This kernel user user-data object
 */
class UserObjectKernel : public Kernel
{
public:
  UserObjectKernel(const std::string & name, InputParameters params);
  virtual ~UserObjectKernel();

protected:
  virtual Real computeQpResidual();

  /// Mutley - do a google search on him if you do not know him
  const MTUserObject & _mutley;
};

#endif /* USEROBJECTKERNEL_H */
