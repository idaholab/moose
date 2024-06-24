//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TopologicalOptimizationTransfer.h"
#include "SamplerTransientMultiApp.h"
#include "SamplerFullSolveMultiApp.h"
#include "Sampler.h"
#include "MooseUtils.h"
#include "pcrecpp.h"

#include "TopologyOptimizerDecision.h"

registerMooseObject("MooseApp", TopologicalOptimizationTransfer);

InputParameters
TopologicalOptimizationTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription(
      "This class transfers the configuration from the topology reporter to the "
      "multiapp system and updates the mesh.");

  params.addRequiredParam<UserObjectName>(
      "decision_reporter",
      "The Reporter of type TopologyOptimizerDecision that makes accept/reject decisions");

  params.addParam<PostprocessorName>(
      "objective_name",
      "The objective function name as defined in the subapp postprocessors block. This "
      "function is the one you would like to transfer its value to the optimizer(sampler).  Note: "
      "This is a "
      "post processor name from your MultiApp's input file!");
  return params;
}

TopologicalOptimizationTransfer::TopologicalOptimizationTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _objective_name(isParamValid("objective_name") ? getParam<PostprocessorName>("objective_name")
                                                   : "")
{
  if (isParamValid("from_multi_app") && !isParamValid("objective_name"))
    mooseError("For from_multi_app transfers objective_name must be provided");
}

void
TopologicalOptimizationTransfer::initialSetup()
{
  // Getting the user object
  auto & uo = _fe_problem.getUserObject<UserObject>(getParam<UserObjectName>("decision_reporter"));
  _reporter = dynamic_cast<TopologyOptimizerDecision *>(&uo);
  if (!_reporter)
    paramError("decision_reporter", "This object must be a 'TopologyOptimizerDecision' object.");
}

void
TopologicalOptimizationTransfer::execute()
{
  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      const auto multi_app = getToMultiApp();
      if (multi_app->numLocalApps() > 1)
        mooseError("This transfer should have at most one local app");
      unsigned int multi_app_id = multi_app->firstLocalApp();

      // get proposed configuration
      std::vector<dof_id_type> proposed_config;
      _reporter->getProposedConfiguration(0, proposed_config);

      // assign proposed configuration
      // do a loop over ALL elements in the mesh; TODO: revisit when parallelizing
      // problem is the offset in index!!
      auto & to_mesh = getToMultiApp()->appProblemBase(multi_app_id).mesh();
      auto begin = to_mesh.getMesh().active_elements_begin();
      auto end = to_mesh.getMesh().active_elements_end();
      dof_id_type index = 0;
      for (Elem * elem : as_range(begin, end))
      {
        elem->subdomain_id() = proposed_config[index];
        ++index;
      }
      to_mesh.meshChanged();
      break;
    }

    case FROM_MULTIAPP:
    {
      const auto multi_app = getFromMultiApp();
      if (multi_app->numLocalApps() > 1)
        mooseError("This transfer should have at most one local app");
      auto val = multi_app->appProblemBase(multi_app->firstLocalApp())
                     .getPostprocessorValueByName(_objective_name);
      _reporter->setProposedObjectiveValues(val);
      break;
    }
  }
}
