//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionInvalidityReporter.h"

#include "SolutionInvalidity.h"

registerMooseObject("MooseApp", SolutionInvalidityReporter);

InputParameters
SolutionInvalidityReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reports the Summary Table of Solution Invalid Counts.");
  return params;
}

SolutionInvalidityReporter::SolutionInvalidityReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _solution_invalidity(declareValueByName<const SolutionInvalidity *>(
        "solution_invalidity", REPORTER_MODE_ROOT, &_app.solutionInvalidity()))
{
}

void
to_json(nlohmann::json & json, const SolutionInvalidity * const & solution_invalidity)
{
  mooseAssert(solution_invalidity.processor_id() == 0, "should only be called on rank 0");
  mooseAssert(solution_invalidity, "solution_invalidity is not set");

  const auto & solution_registry = moose::internal::getSolutionInvalidityRegistry();
  auto count = solution_invalidity->counts();
  for (const auto id : index_range(count))
  {
    nlohmann::json entry;
    entry["object_type"] = solution_registry.item(id).object_type;
    entry["latest_counts"] = count[id].counts;
    entry["timestep_counts"] = count[id].timeiter_counts;
    entry["total_counts"] = count[id].total_counts;
    json.push_back(entry);
  }
}
