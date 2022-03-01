//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * Class for output information regarding Controls to the screen
 */
class PerfGraphOutput : public Output
{
public:
  static InputParameters validParams();

  PerfGraphOutput(const InputParameters & parameters);

protected:
  virtual bool shouldOutput(const ExecFlagType & type) override;
  virtual void output(const ExecFlagType & type) override;

  // Detail level
  unsigned int _level;

  bool _heaviest_branch;

  unsigned int _heaviest_sections;
};
