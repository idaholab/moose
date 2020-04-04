//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InclinedNoDisplacementBCAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("TensorMechanicsApp", InclinedNoDisplacementBCAction, "add_bc");

InputParameters
InclinedNoDisplacementBCAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up inclined no displacement boundary conditions");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where the pressure will be applied");

  params.addParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::vector<AuxVariableName>>("save_in", "The displacement residuals");

  params.addRequiredParam<Real>("penalty", "Penalty parameter");
  return params;
}

InclinedNoDisplacementBCAction::InclinedNoDisplacementBCAction(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _save_in(getParam<std::vector<AuxVariableName>>("save_in"))
{
  if (_ndisp == 1)
    mooseError("InclinedNoDisplacementBC is specific to 2D and 3D models.");

  if (_save_in.size() != 0 && _save_in.size() != _ndisp)
    mooseError("Number of save_in variables should equal to the number of displacement variables ",
               _displacements.size());
}

void
InclinedNoDisplacementBCAction::act()
{
  const std::string kernel_name = "PenaltyInclinedNoDisplacementBC";

  // Create pressure BCs
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    // Create unique kernel name for each of the components
    std::string unique_kernel_name = kernel_name + "_" + _name + "_" + Moose::stringify(i);

    InputParameters params = _factory.getValidParams(kernel_name);
    params.applyParameters(parameters());
    params.set<bool>("use_displaced_mesh") = false;
    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = _displacements[i];

    if (_save_in.size() == _ndisp)
      params.set<std::vector<AuxVariableName>>("save_in") = {_save_in[i]};

    _problem->addBoundaryCondition(kernel_name, unique_kernel_name, params);
  }
}
