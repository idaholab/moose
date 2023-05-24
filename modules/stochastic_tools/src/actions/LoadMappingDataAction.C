//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LoadMappingDataAction.h"
#include "VariableMappingBase.h"
#include "FEProblem.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

registerMooseAction("StochasticToolsApp", LoadMappingDataAction, "load_mapping_data");

InputParameters
LoadMappingDataAction::validParams()
{
  InputParameters params = LoadModelDataAction<VariableMappingBase>::validParams();
  params.addClassDescription(
      "Load the model data for the objects defined in the `[VariableMappings]` block.");
  return params;
}

LoadMappingDataAction::LoadMappingDataAction(const InputParameters & params)
  : LoadModelDataAction<VariableMappingBase>(params)
{
}
