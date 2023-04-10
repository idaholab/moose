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
class MappingBase;

/**
 * Action for loading the model data for the mapping objects
 */
class LoadMappingDataAction : public Action
{
public:
  static InputParameters validParams();
  LoadMappingDataAction(const InputParameters & params);
  virtual void act() override;

private:
  /**
   * Load the necessary information for the given model
   * @param mapping Reference to the Mapping object whose data shall be loaded
   */
  void load(const MappingBase & mapping);
};
