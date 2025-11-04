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
#include "SolutionInvalidity.h"

/**
 * Class to output the solution invalidity history summary to console
 */
class SolutionInvalidityOutput : public Output
{
public:
  static InputParameters validParams();

  SolutionInvalidityOutput(const InputParameters & parameters);

protected:
  virtual bool shouldOutput() override;

  virtual void output() override;

  /// @brief define The number of time steps to group together in the table reporting the solution invalidity occurrences.
  unsigned int _timestep_interval;

  /// @brief get SolutionInvalidity reference
  SolutionInvalidity & _solution_invalidity;
};
