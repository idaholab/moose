//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

class SolutionInvalidity;

/**
 * Reports the summary table of solution invalid warnings
 */
class SolutionInvalidityReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  SolutionInvalidityReporter(const InputParameters & parameters);

  void initialize() override {}
  void finalize() override {}
  void execute() override {}

private:
  const SolutionInvalidity * const & _solution_invalidity;
};

void to_json(nlohmann::json & json, const SolutionInvalidity & solution_invalidity);
