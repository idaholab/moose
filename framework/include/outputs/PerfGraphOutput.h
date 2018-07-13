//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PERFGRAPHOUTPUT_H
#define PERFGRAPHOUTPUT_H

// MOOSE includes
#include "Output.h"

// Forward declarations
class PerfGraphOutput;

template <>
InputParameters validParams<PerfGraphOutput>();

/**
 * Class for output information regarding Controls to the screen
 */
class PerfGraphOutput : public Output
{
public:
  /**
   * Class constructor
   */
  PerfGraphOutput(const InputParameters & parameters);

protected:
  /**
   * Perform the output of control information
   */
  virtual void output(const ExecFlagType & type) override;

  // Detail level
  unsigned int _level;

  bool _heaviest_branch;

  unsigned int _heaviest_sections;
};

#endif /* PERFGRAPHOUTPUT_H */
