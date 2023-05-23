//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "LoadModelDataAction.h"
#include "SurrogateModel.h"

/**
 * Action which is responsible for loading essential data for surrogates from
 * separate binary files.
 */
class LoadSurrogateDataAction : public LoadModelDataAction<SurrogateModel>
{
public:
  static InputParameters validParams();

  LoadSurrogateDataAction(const InputParameters & params);
};
