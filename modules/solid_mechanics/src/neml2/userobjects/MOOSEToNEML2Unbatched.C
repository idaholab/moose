//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEToNEML2Unbatched.h"

InputParameters
MOOSEToNEML2Unbatched::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params += GeneralUserObject::validParams();

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

MOOSEToNEML2Unbatched::MOOSEToNEML2Unbatched(const InputParameters & params)
  : MOOSEToNEML2(params), GeneralUserObject(params)
{
}
