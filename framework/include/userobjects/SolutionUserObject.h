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
#include "SolutionUserObjectBase.h"
#include "FunctionParserUtils.h"

/**
 * User object that reads an existing solution from an input file and
 * uses it in the current simulation.
 */
class SolutionUserObject : public SolutionUserObjectBase, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  SolutionUserObject(const InputParameters & parameters);

  /**
   * Get the time at which to sample the solution
   */
  virtual Real solutionSampleTime() override;

  /// function parser object for transforming the solution sample time
  SymFunctionPtr _time_transformation;
};
