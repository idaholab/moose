//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEToNEML2Batched.h"

InputParameters
MOOSEToNEML2Batched::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params += ElementUserObject::validParams();

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

MOOSEToNEML2Batched::MOOSEToNEML2Batched(const InputParameters & params)
  : MOOSEToNEML2(params), ElementUserObject(params)
{
}

#ifdef NEML2_ENABLED
void
MOOSEToNEML2Batched::initialize()
{
  _buffer.clear();
}

void
MOOSEToNEML2Batched::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    _buffer.push_back(convertQpMOOSEData());
}

void
MOOSEToNEML2Batched::threadJoin(const UserObject & uo)
{
  // append vectors
  const auto & m2n = static_cast<const MOOSEToNEML2Batched &>(uo);
  _buffer.insert(_buffer.end(), m2n._buffer.begin(), m2n._buffer.end());
}

neml2::Tensor
MOOSEToNEML2Batched::gatheredData() const
{
  return neml2::Tensor(torch::stack(_buffer, 0), 1);
}
#endif
