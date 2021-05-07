//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVFlowBC.h"

class InputParameters;

/**
 * A parent class for INSFV fully developed flow boundary conditions
 */
class INSFVFullyDevelopedFlowBC : public INSFVFlowBC
{
public:
  static InputParameters validParams();
  INSFVFullyDevelopedFlowBC(const InputParameters & params);
};
