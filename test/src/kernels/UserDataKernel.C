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

#include "UserDataKernel.h"

template<>
InputParameters validParams<UserDataKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("user_data", "The name of user data object to use.");
  return params;
}

UserDataKernel::UserDataKernel(const std::string & name, InputParameters params) :
    Kernel(name, params),
    _mutley(dynamic_cast<const MTUserData &>(getUserData("user_data")))   // get user-data object and cast it down so we can use it
{
}

UserDataKernel::~UserDataKernel()
{
}

Real
UserDataKernel::computeQpResidual()
{
  Real val = _mutley.doSomething();     // let Mutley do something
  return -_test[_i][_qp] * val;
}
