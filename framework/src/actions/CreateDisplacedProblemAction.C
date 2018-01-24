/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CreateDisplacedProblemAction.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

template <>
InputParameters
validParams<CreateDisplacedProblemAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<std::string>>(
      "displacements",
      "The variables corresponding to the x y z displacements of the mesh.  If "
      "this is provided then the displacements will be taken into account during "
      "the computation.");

  return params;
}

CreateDisplacedProblemAction::CreateDisplacedProblemAction(InputParameters parameters)
  : Action(parameters)
{
}

void
CreateDisplacedProblemAction::act()
{
  if (isParamValid("displacements"))
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
