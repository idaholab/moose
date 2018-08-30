//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateProblemDefaultAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "EigenProblem.h"
#include "MooseApp.h"
#include "CreateExecutionerAction.h"

registerMooseAction("MooseApp", CreateProblemDefaultAction, "create_problem_default");
registerMooseAction("MooseApp", CreateProblemDefaultAction, "determine_system_type");

template <>
InputParameters
validParams<CreateProblemDefaultAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<bool>("solve",
                        true,
                        "Whether or not to actually solve the Nonlinear system.  "
                        "This is handy in the case that all you want to do is "
                        "execute AuxKernels, Transfers, etc. without actually "
                        "solving anything");
  return params;
}

CreateProblemDefaultAction::CreateProblemDefaultAction(InputParameters parameters)
  : Action(parameters)
{
}

void
CreateProblemDefaultAction::act()
{
  if (_current_task == "determine_system_type")
  {
    // Determine whether the Executioner is derived from EigenExecutionerBase and
    // set a flag on MooseApp that can be used during problem construction.
    bool use_nonlinear = true;
    bool use_eigenvalue = false;
    auto p = _awh.getActionByTask<CreateExecutionerAction>("setup_executioner");
    if (p)
    {
      auto & exparams = p->getObjectParams();
      use_nonlinear = !(exparams.isParamValid("_eigen") && exparams.get<bool>("_eigen"));
      use_eigenvalue =
          (exparams.isParamValid("_use_eigen_value") && exparams.get<bool>("_use_eigen_value"));
    }

    _app.useNonlinear() = use_nonlinear;
    _app.useEigenvalue() = use_eigenvalue;
    return;
  }

  // act only if we have mesh
  if (_mesh.get() != NULL)
  {
    // Make sure the problem hasn't already been created elsewhere
    if (!_problem)
    {
      std::string type;
      if (_app.useEigenvalue())
        type = "EigenProblem";
      else
        type = "FEProblem";
      auto params = _factory.getValidParams(type);

      params.set<MooseMesh *>("mesh") = _mesh.get();
      params.set<bool>("use_nonlinear") = _app.useNonlinear();
      params.set<bool>("solve") = getParam<bool>("solve");

      _problem = _factory.create<FEProblemBase>(type, "MOOSE Problem", params);
    }
    else
    {
      // otherwise perform necessary sanity checks
      if (_app.useEigenvalue() && !(std::dynamic_pointer_cast<EigenProblem>(_problem)))
        mooseError("Problem has to be of a EigenProblem (or derived subclass) type when using "
                   "eigen executioner");
    }
  }
}
