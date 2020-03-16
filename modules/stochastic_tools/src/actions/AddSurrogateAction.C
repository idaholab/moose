//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSurrogateAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "UserObject.h"

registerMooseAction("StochasticToolsApp", AddSurrogateAction, "add_surrogate");

InputParameters
AddSurrogateAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Adds SurrogateModel objects contained within the `[Surrogates]` input block.");
  return params;
}

AddSurrogateAction::AddSurrogateAction(InputParameters params) : MooseObjectAction(params) {}

void
AddSurrogateAction::act()
{
  // If the 'filename' parameter is supplied then the model is NOT being trained:
  //    (1) the 'execute_on' is set to NONE automatically; if the user has set
  //        the 'execute_on' parameter produce a warning.
  //    (2) if any training parameters are set then produce a warning that the model is being
  //        evaluated but training parameters are set
  //
  // If the 'filename' parameters is not set then the model IS being trained
  //    (1) check that all 'training' parameters are valid and error if they are not; this
  //        allows all training parameters to be optional
  bool warn_execute_on = false;           // use a flag so a paramWarning can be called
  std::set<std::string> warn_no_training; // "training" parameters set, but not training
  std::set<std::string> err_yes_training; // "training" parameters not valid, but training
  if (_moose_object_pars.isParamValid("filename"))
  {
    if (!_moose_object_pars.get<ExecFlagEnum>("execute_on").contains("NONE"))
      warn_execute_on = _moose_object_pars.isParamSetByUser("execute_on");
    _moose_object_pars.set<ExecFlagEnum>("execute_on") = "NONE";

    // Loop through all parameters in "training" group, if they are set then warning as well
    for (const std::string & name : _moose_object_pars.getGroupParameters("training"))
      if (_moose_object_pars.isParamSetByUser(name))
        warn_no_training.emplace(name);
  }
  else
  {
    for (const std::string & name : _moose_object_pars.getGroupParameters("training"))
      if (!_moose_object_pars.isParamValid(name))
        err_yes_training.emplace(name);
  }

  // Create the actual object
  _problem->addUserObject(_type, _name, _moose_object_pars);

  if (warn_execute_on)
  {
    const UserObject & obj = _problem->getUserObjectBase(_name);
    obj.paramWarning("execute_on",
                     "The '",
                     obj.name(),
                     "' model is setup for evaluation because the 'filename' parameter has been "
                     "set. Howerver, the 'execute_on' parameter has also been set. When a model is "
                     "setup for evaluation the execution of the object is automatically disabled "
                     "as it is expected that the evaluate method is being used directly.");
  }

  if (!warn_no_training.empty())
  {
    const UserObject & obj = _problem->getUserObjectBase(_name);
    for (const std::string & name : warn_no_training)
      obj.paramWarning(name,
                       "The '",
                       obj.name(),
                       "' model is setup for evaluation because the 'filename' parameter has been "
                       "set. However, the training parameter '",
                       name,
                       "' is also defined. Training parameters should not be set when the model is "
                       "setup for evaluation.");
  }

  if (!err_yes_training.empty())
  {
    const UserObject & obj = _problem->getUserObjectBase(_name);
    for (const std::string & name : err_yes_training)
      obj.paramWarning(name,
                       "The '",
                       obj.name(),
                       "' model is setup for training but the training parameter '",
                       name,
                       "' is missing or invalid.");

    ::mooseError("The '",
                 obj.name(),
                 "' model is setup for training but one or more training parameters are missing or "
                 "invalid.");
  }
}
