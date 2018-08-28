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
#include "NonlinearSystemBase.h"
#include "MooseApp.h"

registerMooseAction("MooseApp", CreateProblemDefaultAction, "create_problem_default");

template <>
InputParameters
validParams<CreateProblemDefaultAction>()
{
  MultiMooseEnum coord_types("XYZ RZ RSPHERICAL", "XYZ");
  MooseEnum rz_coord_axis("X=0 Y=1", "Y");

  InputParameters params = validParams<MooseObjectAction>();
  params.set<std::string>("type") = "FEProblem";
  params.addParam<std::string>("name", "MOOSE Problem", "The name the problem");
  return params;
}

CreateProblemDefaultAction::CreateProblemDefaultAction(InputParameters parameters)
  : MooseObjectAction(parameters)
{
}

void
CreateProblemDefaultAction::act()
{
  // build the problem only if we have mesh
  if (_mesh.get() != NULL)
  {
    // Make sure the problem hasn't already been created elsewhere
    if (!_problem)
    {
      _moose_object_pars.set<MooseMesh *>("mesh") = _mesh.get();
      _moose_object_pars.set<bool>("use_nonlinear") = _app.useNonlinear();

      _problem =
          _factory.create<FEProblemBase>(_type, getParam<std::string>("name"), _moose_object_pars);
      if (!_problem)
        mooseError("Problem has to be of a FEProblemBase type");

      if (_app.useEigenvalue() && _type != "EigenProblem" &&
          !(std::dynamic_pointer_cast<EigenProblem>(_problem)))
        mooseError("Problem has to be of a EigenProblem (or derived subclass) type when using "
                   "eigen executioner");

      MultiMooseEnum coord_types("XYZ RZ RSPHERICAL", "XYZ");
      _problem->setCoordSystem(std::vector<SubdomainName>(), coord_types);
    }

    // If the problem exists, perform a sanity check on the types
    else if (_moose_object_pars.isParamSetByUser("type") &&
             _problem->type() != _moose_object_pars.get<std::string>("type"))
      mooseError("User requested problem type conflicts with existing Problem type: ",
                 _problem->type(),
                 " and ",
                 _moose_object_pars.get<std::string>("type"));
  }
}
