//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMProblem.h"
#include "AddPostprocessorAction.h"

registerMooseObject("ThermalHydraulicsApp", THMProblem);

InputParameters
THMProblem::validParams()
{
  InputParameters params = FEProblem::validParams();

  params.addParam<bool>("2nd_order_mesh", false, "Use 2nd order elements in the mesh");

  params.addParam<FileName>("initial_from_file",
                            "The name of an exodus file with initial conditions");
  params.addParam<std::string>(
      "initial_from_file_timestep",
      "LATEST",
      "Gives the timestep (or \"LATEST\") for which to read a solution from a file "
      "for a given variable. (Default: LATEST)");

  params.addClassDescription("Specialization of FEProblem to run with component subsystem");

  return params;
}

THMProblem::THMProblem(const InputParameters & parameters)
  : FEProblem(parameters), Simulation(*this, parameters)
{
}

void
THMProblem::advanceState()
{
  FEProblem::advanceState();
  Simulation::advanceState();
}

void
THMProblem::copySolutionsBackwards()
{
  FEProblem::copySolutionsBackwards();
  Simulation::advanceState();
}

bool
THMProblem::hasPostprocessor(const std::string & name) const
{
  if (_reporter_data.hasReporterValue(ReporterName(name, "value")))
    return true;

  // Sometimes we want to know if a Postprocessor exists well before Postprocessor objects
  // are actually constructed (within check() in Components) in order to provide a better
  // error than MOOSE (which checks for Postprocessor existence well after objects are added).
  // Therefore, if we can't find that we have a Reporter value that represents the PP,
  // this method might have been called before the add_postprocessor task that sets up the
  // Reporter values for the postprocessors. Therefore, let's also see which Postprocessors
  // are slated to be constructed in the future (via the AddPostprocessorAction actions,
  // in which the names are the PP names themselves)
  for (const auto & action : getMooseApp().actionWarehouse().getActions<AddPostprocessorAction>())
    if (action->name() == name)
      return true;

  return false;
}
