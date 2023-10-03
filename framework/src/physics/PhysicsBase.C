//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsBase.h"
#include "MooseUtils.h"

InputParameters
PhysicsBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription("Creates all the objects necessary to solve a particular physics");

  MooseEnum transient_options("true false same_as_problem", "same_as_problem");
  params.addParam<MooseEnum>(
      "transient", transient_options, "Whether the physics is to be solved as a transient");

  params.transferParam<std::vector<SubdomainName>>(BlockRestrictable::validParams(), "block");

  return params;
}

// InputParameters
// PhysicsBase::selectParams(const InputParameters & parameters,
//                           std::vector<std::string> & params_to_keep)
// {
//   InputParameters params = emptyInputParameters();
//   for (const auto & param : params_to_keep)
//     if (!parameters.have_parameter(param))
//       mooseError(
//           "Cannot transfer parameter", param, "because it does not exist in the source object");
//   // params += parameters;
// }

PhysicsBase::PhysicsBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    BlockRestrictable(this),
    _is_transient(getParam<MooseEnum>("transient"))
{
  _problem = getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");
  _factory = &_app.getFactory();

  if (_is_transient == "true" && !getProblem().isTransient())
    paramError("transient", "We cannot solve a physics as transient in a steady problem");
}

bool
PhysicsBase::isTransient() const
{
  if (_is_transient == "true")
    return true;
  else if (_is_transient == "false")
    return false;
  else
    return getProblem().isTransient();
}
