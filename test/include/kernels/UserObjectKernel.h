//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef USEROBJECTKERNEL_H
#define USEROBJECTKERNEL_H

#include "Kernel.h"
#include "MTUserObject.h"

class UserObjectKernel;

template <>
InputParameters validParams<UserObjectKernel>();

/**
 * This kernel user user-data object
 */
class UserObjectKernel : public Kernel
{
public:
  UserObjectKernel(const InputParameters & params);
  virtual ~UserObjectKernel();

protected:
  virtual Real computeQpResidual();

  /// Mutley - do a google search on him if you do not know him
  const MTUserObject & _mutley;
};

#endif /* USEROBJECTKERNEL_H */
