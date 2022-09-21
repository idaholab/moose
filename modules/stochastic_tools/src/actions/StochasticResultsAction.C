//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StochasticResultsAction.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "Transfer.h"
#include "SetupMeshAction.h"
#include "SamplerPostprocessorTransfer.h"
#include "StochasticResults.h"

registerMooseAction("StochasticToolsApp",
                    StochasticResultsAction,
                    "declare_stochastic_results_vectors");

InputParameters
StochasticResultsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action for performing initialization of StochasticResults vectors "
                             "based on SamplerPostprocessorTransfer.");
  return params;
}

StochasticResultsAction::StochasticResultsAction(const InputParameters & params) : Action(params) {}

void
StochasticResultsAction::act()
{
  if (_current_task == "declare_stochastic_results_vectors")
  {
    for (std::shared_ptr<Transfer> & transfer_ptr :
         _problem->getTransfers(Transfer::DIRECTION::FROM_MULTIAPP))
    {
      auto ptr = std::dynamic_pointer_cast<SamplerPostprocessorTransfer>(transfer_ptr);
      if (ptr != nullptr)
      {
        const auto & result_name =
            ptr->getParam<VectorPostprocessorName>("to_vector_postprocessor");
        const std::vector<VectorPostprocessorName> & vpp_names = ptr->vectorNames();

        // Get the StochasticResults storage object, get it by base class to allow for better
        // type check error message
        auto & uo = _problem->getUserObject<UserObject>(result_name);
        auto * results = dynamic_cast<StochasticResults *>(&uo);
        if (!results)
          mooseError("The object prescribed by the 'to_vector_postprocessor' parameter in ",
                     ptr->name(),
                     " must be a 'StochasticResults' object.");
        for (const auto & vpp_name : vpp_names)
          results->initVector(vpp_name);
      }
    }
  }
}
