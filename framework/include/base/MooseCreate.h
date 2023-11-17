//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "InputParameters.h"

/**
 * Instanitiate object for any MOOSE-based application
 *
 * This object must be created in the main() of any MOOSE-based application so
 * everything is properly initialized and finalized.
 */
class MooseCreate
{
public:
  MooseCreate(std::string app_name, int argc, char * argv[]);
  virtual ~MooseCreate() = default;

  void addParam(InputParameters & params);
  std::shared_ptr<MooseApp> getApp() { return _app; };

private:
  std::shared_ptr<MooseApp> _app;
};
