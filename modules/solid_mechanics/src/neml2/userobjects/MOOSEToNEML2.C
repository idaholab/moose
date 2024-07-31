//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEToNEML2.h"
#include "NEML2Utils.h"

#ifndef NEML2_ENABLED
NEML2ObjectStubImplementation(MOOSEToNEML2, ElementUserObject);
#else

#include "neml2/misc/math.h"
#include "MooseUtils.h"

InputParameters
MOOSEToNEML2::validParams()
{
  auto params = ElementUserObject::validParams();

  params.addRequiredParam<std::string>("neml2_variable", "Name of the NEML2 variable to write to");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

MOOSEToNEML2::MOOSEToNEML2(const InputParameters & params)
  : ElementUserObject(params),
    _neml2_variable(
        neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_variable")))
{
}

void
MOOSEToNEML2::insertIntoInput(neml2::LabeledVector & input) const
{
  input.base_index_put_(_neml2_variable, neml2::Tensor(torch::stack(_buffer, 0), 1));
}

void
MOOSEToNEML2::initialize()
{
  _buffer.clear();
}

void
MOOSEToNEML2::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    _buffer.push_back(convertQpMOOSEData());
}

void
MOOSEToNEML2::threadJoin(const UserObject & uo)
{
  // append vectors
  const auto & m2n = static_cast<const MOOSEToNEML2 &>(uo);
  _buffer.insert(_buffer.end(), m2n._buffer.begin(), m2n._buffer.end());
}

#endif
