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
#include "VariableMappingBase.h"

/**
 * Action which is responsible for loading essential data for variable mapping
 * objects from separate binary files.
 */
class LoadMappingDataAction : public LoadModelDataAction<VariableMappingBase>
{
public:
  static InputParameters validParams();

  LoadMappingDataAction(const InputParameters & params);
};
