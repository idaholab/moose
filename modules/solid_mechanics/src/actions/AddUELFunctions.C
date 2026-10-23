//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddUELFunctions.h"
#include "Factory.h"
#include "FEProblem.h"
#include "AbaqusUELMesh.h"

registerMooseAction("SolidMechanicsApp", AddUELFunctions, "add_function");

InputParameters
AddUELFunctions::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Add a PiecewiseLinear function for each *Amplitude table in an Abaqus input");
  return params;
}

AddUELFunctions::AddUELFunctions(const InputParameters & params) : Action(params) {}

void
AddUELFunctions::act()
{
  const auto uel_mesh = std::dynamic_pointer_cast<AbaqusUELMesh>(_mesh);
  if (!uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");

  for (const auto & [name, amplitude] : uel_mesh->getModel()._amplitudes)
  {
    auto func_params = _factory.getValidParams("PiecewiseLinear");
    func_params.set<std::vector<Real>>("x") = amplitude._time;
    func_params.set<std::vector<Real>>("y") = amplitude._value;
    _problem->addFunction("PiecewiseLinear", Abaqus::amplitudeFunctionName(name), func_params);
  }
}
