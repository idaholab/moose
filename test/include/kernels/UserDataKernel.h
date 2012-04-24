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

#ifndef USERDATAKERNEL_H
#define USERDATAKERNEL_H

#include "Kernel.h"
#include "MTUserData.h"

class UserDataKernel;

template<>
InputParameters validParams<UserDataKernel>();

/**
 * This kernel user user-data object
 */
class UserDataKernel : public Kernel
{
public:
  UserDataKernel(const std::string & name, InputParameters params);
  virtual ~UserDataKernel();

protected:
  virtual Real computeQpResidual();

  /// Mutley - do a google search on him if you do not know him
  const MTUserData & _mutley;
};

#endif /* USERDATAKERNEL_H */
