//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEToNEML2Parameter.h"
#include "NEML2Utils.h"

#ifndef NEML2_ENABLED
#define MOOSEToNEML2ParameterStub(name)                                                            \
  NEML2ObjectStubImplementationOpen(name, ElementUserObject);                                      \
  NEML2ObjectStubParam(std::string, "neml2_parameter");                                            \
  NEML2ObjectStubImplementationClose(name, ElementUserObject)
MOOSEToNEML2ParameterStub(MOOSEToNEML2Parameter);
#else

#include "neml2/misc/math.h"
#include "MooseUtils.h"

InputParameters
MOOSEToNEML2Parameter::validParams()
{
  auto params = ElementUserObject::validParams();

  params.addRequiredParam<std::string>("neml2_parameter",
                                       "Name of the NEML2 parameter to write to");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

MOOSEToNEML2Parameter::MOOSEToNEML2Parameter(const InputParameters & params)
  : ElementUserObject(params), _neml2_parameter(getParam<std::string>("neml2_parameter"))
{
}

void
MOOSEToNEML2Parameter::insertIntoParameter(neml2::Model & model) const
{
  model.set_parameter(_neml2_parameter, neml2::Tensor(torch::stack(_buffer, 0), 1));
}

void
MOOSEToNEML2Parameter::initialize()
{
  _buffer.clear();
}

void
MOOSEToNEML2Parameter::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    _buffer.push_back(convertQpMOOSEData());
}

void
MOOSEToNEML2Parameter::threadJoin(const UserObject & uo)
{
  // append vectors
  const auto & m2n = static_cast<const MOOSEToNEML2Parameter &>(uo);
  _buffer.insert(_buffer.end(), m2n._buffer.begin(), m2n._buffer.end());
}

#endif
