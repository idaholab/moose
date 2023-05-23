//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LoadSurrogateDataAction.h"
#include "SurrogateModel.h"
#include "FEProblem.h"
#include "RestartableDataIO.h"
#include "StochasticToolsApp.h"

registerMooseAction("StochasticToolsApp", LoadSurrogateDataAction, "load_surrogate_data");

InputParameters
LoadSurrogateDataAction::validParams()
{
  InputParameters params = LoadModelDataAction<SurrogateModel>::validParams();
  params.addClassDescription("Calls load method on SurrogateModel objects contained within the "
                             "`[Surrogates]` input block, if a filename is given.");
  return params;
}

LoadSurrogateDataAction::LoadSurrogateDataAction(const InputParameters & params)
  : LoadModelDataAction<SurrogateModel>(params)
{
}
