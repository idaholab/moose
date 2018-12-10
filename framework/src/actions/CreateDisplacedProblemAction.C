//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateDisplacedProblemAction.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

registerMooseAction("MooseApp", CreateDisplacedProblemAction, "init_displaced_problem");

template <>
InputParameters
validParams<CreateDisplacedProblemAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<std::string>>(
      "displacements",
      "The variables corresponding to the x y z displacements of the mesh.  If "
      "this is provided then the displacements will be taken into account during "
      "the computation. Creation of the displaced mesh can be suppressed even if "
      "this is set by setting 'use_displaced_mesh = false'.");
  params.addParam<bool>(
      "use_displaced_mesh",
      true,
      "Create the displaced mesh if the 'displacements' "
      "parameter is set. If this is 'false', a displaced mesh will not be created, "
      "regardless of whether 'displacements' is set.");

  return params;
}

CreateDisplacedProblemAction::CreateDisplacedProblemAction(InputParameters parameters)
  : Action(parameters)
{
}

void
CreateDisplacedProblemAction::act()
{
  if (isParamValid("displacements") && getParam<bool>("use_displaced_mesh"))
  {
    if (!_displaced_mesh)
      mooseError("displacements were set but a displaced mesh wasn't created!");

    // Define the parameters
    InputParameters object_params = _factory.getValidParams("DisplacedProblem");
    object_params.set<std::vector<std::string>>("displacements") =
        getParam<std::vector<std::string>>("displacements");
    object_params.set<MooseMesh *>("mesh") = _displaced_mesh.get();
    object_params.set<FEProblemBase *>("_fe_problem_base") = _problem.get();

    // Create the object
    std::shared_ptr<DisplacedProblem> disp_problem =
        _factory.create<DisplacedProblem>("DisplacedProblem", "DisplacedProblem", object_params);

    // Add the Displaced Problem to FEProblemBase
    _problem->addDisplacedProblem(disp_problem);
  }
}
