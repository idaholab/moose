//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicalBodyForceAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("TensorMechanicsApp", MechanicalBodyForceAction, "add_kernel");

InputParameters
MechanicalBodyForceAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up Pressure boundary conditions");

  params.addParam<std::vector<SubdomainName>>("block",
                                              "The block ids where the body force will be applied");

  params.addParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addParam<FunctionName>(
      "function", "1", "Function to scale the coupled body force vector property");
  params.addParam<Real>(
      "hht_alpha", 0.0, "alpha parameter required for HHT time integration scheme");
  params.addRequiredParam<MaterialPropertyName>("body_force", "Force per unit volume vector");
  return params;
}

MechanicalBodyForceAction::MechanicalBodyForceAction(const InputParameters & params)
  : Action(params)
{
}

void
MechanicalBodyForceAction::act()
{
  std::string kernel_type = "MechanicalBodyForce";

  auto displacements = getParam<std::vector<VariableName>>("displacements");
  for (const auto & disp : displacements)
  {
    InputParameters params = _factory.getValidParams(kernel_type);
    params.applyParameters(parameters());
    params.set<NonlinearVariableName>("variable") = disp;
    _problem->addKernel(kernel_type, _name + "_" + disp, params);
  }
}
