//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObject.h"
#include "MooseObjectAction.h"
#include "MooseUtils.h"
#include "Factory.h"
#include "Conversion.h"
#include "MooseMesh.h"
#include "MooseApp.h"
#include "EigenExecutionerBase.h"

InputParameters
MooseObjectAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<std::string>(
      "type", "A string representing the Moose Object that will be built by this Action");
  params.addParam<bool>("isObjectAction", true, "Indicates that this is a MooseObjectAction.");
  params.addParamNamesToGroup("isObjectAction", "Advanced");
  params.addClassDescription("Base class for all the actions creating a MOOSE object");
  return params;
}

MooseObjectAction::MooseObjectAction(const InputParameters & params)
  : Action(params),
    _type(getParam<std::string>("type")),
    // We will create a second parameters object from the main factory unless instructed otherwise
    _moose_object_pars(!params.have_parameter<bool>("skip_param_construction") ||
                               (params.have_parameter<bool>("skip_param_construction") &&
                                !params.get<bool>("skip_param_construction"))
                           ? _factory.getValidParams(_type)
                           : MooseObject::validParams())
{
  _moose_object_pars.blockFullpath() = params.blockFullpath();
}

void
MooseObjectAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  addRelationshipManagers(input_rm_type, _moose_object_pars);
}

void
MooseObjectAction::timedAct()
{
  TIME_SECTION(_act_timer);

  // we reset the default values of execute_on parameter for objects
  // when legacy fixed-point iteration execute_on is not used
  // Note: this code should be removed once the legacy flag is removed
  //       and the default values should be set in validParams of individual
  //       objects.
  if (!_app.parameters().get<bool>("use_legacy_fixed_point_execute_on"))
  {
    // This resetting should only for objects that have the execute_on parameter and
    // the parameter is not private and is not changed by users.
    // Note: executioners based on EigenExecutionerBase do not have fixed-point iteration,
    //       thus we need to skip the resetting.
    if (_moose_object_pars.have_parameter<ExecFlagEnum>("execute_on") &&
        !_moose_object_pars.isParamSetByUser("execute_on") &&
        !_moose_object_pars.isPrivate("execute_on") &&
        !dynamic_cast<EigenExecutionerBase *>(_app.getExecutioner()))
    {
      // The resetting must be in the quiet mode, which is important for Outputs
      // to allow FEProblemBase::addOutput to override this parameter
      auto & exec_enum = _moose_object_pars.set<ExecFlagEnum>("execute_on", true);
      exec_enum.findReplaceFlag(EXEC_TIMESTEP_BEGIN, EXEC_MULTIAPP_FIXED_POINT_BEGIN);

      // The output should still be defaulting to timestep end
      if (_moose_object_pars.getBase().value_or("") != "Output")
        exec_enum.findReplaceFlag(EXEC_TIMESTEP_END, EXEC_MULTIAPP_FIXED_POINT_END);
    }
  }

  act();
}
