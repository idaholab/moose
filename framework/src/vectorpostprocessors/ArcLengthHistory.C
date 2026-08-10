//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArcLengthHistory.h"
#include "ArcLengthProblem.h"

#include "libmesh/int_range.h"

registerMooseObject("MooseApp", ArcLengthHistory);

InputParameters
ArcLengthHistory::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addClassDescription(
      "Records the load parameter and a set of postprocessor values at every arc-length "
      "continuation increment.");

  params.addRequiredParam<std::vector<PostprocessorName>>(
      "postprocessors", "The postprocessors sampled at every arc-length continuation increment");

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum = {EXEC_ARC_LENGTH_INCREMENT};
  params.setDocString("execute_on", exec_enum.getDocString());

  // Every row of the path is kept, so the history is never reset between increments
  params.set<bool>("contains_complete_history") = true;
  params.suppressParameter<bool>("contains_complete_history");

  return params;
}

ArcLengthHistory::ArcLengthHistory(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _arc_length_problem(dynamic_cast<const ArcLengthProblem *>(&_fe_problem)),
    _pp_names(getParam<std::vector<PostprocessorName>>("postprocessors")),
    _increment(declareVector("increment")),
    _lambda(declareVector("lambda"))
{
  if (!_arc_length_problem)
    mooseError("The recorded history comes from the arc-length problem, so this vector "
               "postprocessor requires [Problem] type = ArcLengthProblem.");

  _pp_values.reserve(_pp_names.size());
  _pp_history.reserve(_pp_names.size());
  for (const auto & pp_name : _pp_names)
  {
    if (pp_name == "increment" || pp_name == "lambda")
      paramError("postprocessors",
                 "The postprocessor '",
                 pp_name,
                 "' cannot be sampled here because the columns 'increment' and 'lambda' are "
                 "already declared for the increment index and the load parameter. Rename the "
                 "postprocessor.");

    _pp_values.push_back(&getPostprocessorValueByName(pp_name));
    _pp_history.push_back(&declareVector(pp_name));
  }
}

void
ArcLengthHistory::initialSetup()
{
  for (const auto & pp_name : _pp_names)
    if (!_fe_problem.getUserObjectBase(pp_name, _tid)
             .getExecuteOnEnum()
             .isValueSet(EXEC_ARC_LENGTH_INCREMENT))
      mooseError("The postprocessor '",
                 pp_name,
                 "' is sampled at every arc-length continuation increment, but it does not execute "
                 "on '",
                 EXEC_ARC_LENGTH_INCREMENT.name(),
                 "'. Add '",
                 EXEC_ARC_LENGTH_INCREMENT.name(),
                 "' to the 'execute_on' parameter of the postprocessor '",
                 pp_name,
                 "'.");
}

void
ArcLengthHistory::initialize()
{
  // no reset/clear needed since this object contains the complete history
}

void
ArcLengthHistory::execute()
{
  // The number of rows already recorded is the index of the increment being recorded now
  _increment.push_back(_increment.size());
  _lambda.push_back(_arc_length_problem->loadParameter());

  for (const auto i : index_range(_pp_values))
    _pp_history[i]->push_back(*_pp_values[i]);
}
