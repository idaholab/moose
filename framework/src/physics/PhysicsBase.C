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

#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"

InputParameters
PhysicsBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription("Creates all the objects necessary to solve a particular physics");

  MooseEnum transient_options("true false same_as_problem", "same_as_problem");
  params.addParam<MooseEnum>(
      "transient", transient_options, "Whether the physics is to be solved as a transient");

  // Restart parameters
  params.addParam<bool>("initialize_variables_from_mesh_file",
                        false,
                        "Determines if the variables that are added by the action are initialized"
                        "from the mesh file (only for Exodus format)");
  params.addParam<std::string>(
      "initial_from_file_timestep",
      "LATEST",
      "Gives the time step number (or \"LATEST\") for which to read the Exodus solution");
  return params;
}

PhysicsBase::PhysicsBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    BlockRestrictable(this),
    _is_transient(getParam<MooseEnum>("transient"))
{
  _problem = getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");
  _factory = &_app.getFactory();
  _dim = _problem->mesh().dimension();

  if (_is_transient == "true" && !getProblem().isTransient())
    paramError("transient", "We cannot solve a physics as transient in a steady problem");

  checkSecondParamSetOnlyIfFirstOneTrue("initialize_variables_from_mesh_file",
                                        "initial_from_file_timestep");
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

void
PhysicsBase::checkParamsBothSetOrNotSet(const std::string & param1,
                                        const std::string & param2) const
{
  if ((isParamValid(param1) + isParamValid(param2)) % 2 != 0)
    paramError(param1,
               "Parameters " + param1 + " and " + param2 +
                   " must be either both set or both unset");
}

void
PhysicsBase::checkSecondParamSetOnlyIfFirstOneTrue(const std::string & param1,
                                                   const std::string & param2) const
{
  mooseAssert(parameters().have_parameter<bool>(param1),
              "Cannot check if parameter " + param1 +
                  " is true if it's not a bool parameter of this object");
  if (!getParam<bool>(param1) && isParamSetByUser(param2))
    paramError(param2,
               "Parameter '" + param1 + "' cannot be set to false if parameter '" + param2 +
                   "' is set by the user");
}

bool
PhysicsBase::nonLinearVariableExists(const VariableName & var_name, bool error_if_aux) const
{
  if (_problem->getNonlinearSystemBase().hasVariable(var_name))
    return true;
  else if (error_if_aux && _problem->getAuxiliarySystem().hasVariable(var_name))
    mooseError("Variable '",
               var_name,
               "' is supposed to be nonlinear for physics '",
               name(),
               "' but it's already defined as auxiliary");
  else
    return false;
}

void
PhysicsBase::checkDependentParameterError(const std::string & main_parameter,
                                          const std::vector<std::string> & dependent_parameters,
                                          const bool should_be_defined) const
{
  for (const auto & param : dependent_parameters)
    if (parameters().isParamSetByUser(param) == !should_be_defined)
      paramError(param,
                 "This parameter should " + std::string(should_be_defined ? "" : "not") +
                     " be given by the user with the corresponding " + main_parameter +
                     " setting!");
}

void
PhysicsBase::assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks) const
{
  // We only set the blocks if we don't have `ANY_BLOCK_ID` defined because the subproblem
  // (through the mesh) errors out if we use this keyword during the addVariable/Kernel
  // functions
  if (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") == blocks.end())
    params.set<std::vector<SubdomainName>>("block") = blocks;
}
