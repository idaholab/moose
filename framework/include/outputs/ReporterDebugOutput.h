//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Output.h"

/**
 * Produces debug output for displaying Reporter information
 */
class ReporterDebugOutput : public Output
{
public:
  static InputParameters validParams();
  ReporterDebugOutput(const InputParameters & parameters);

protected:
  void output() override;
};
