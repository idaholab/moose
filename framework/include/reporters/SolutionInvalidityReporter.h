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
};

// Store solution invalid warnings to a json file
void to_json(nlohmann::json & json, const SolutionInvalidity * const & solution_invalidity);

/**
 * Store and load methods for const SolutionInvalidity *, used in the SolutionInvalidityReporter,
 * which does nothing.
 *
 * We need not do anything here because the data store/load capability of the SolutionInvalidity
 * (non-pointer) object is specialized. The store/load capability of that object will properly
 * initialize what the reporter value in SolutionInvalidityReporter points to.
 */
///@{
void
dataStore(std::ostream &, const SolutionInvalidity *&, void *)
{
}
void
dataLoad(std::istream &, const SolutionInvalidity *&, void *)
{
}
///@}
