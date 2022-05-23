//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FullSolveMultiApp.h"
#include "Transient.h"
#include "CommandLine.h"

// libMesh
#include "libmesh/mesh_tools.h"

class OverrideCliArgs : public FullSolveMultiApp
{
public:
  static InputParameters validParams();
  OverrideCliArgs(const InputParameters & parameters);

protected:
  /// function that provides cli_args to subapps
  virtual std::vector<std::string> cliArgs() const override;
};
