//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ElementIDOutputAction.h"
#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "AddOutputAction.h"

registerMooseAction("MooseApp", ElementIDOutputAction, "add_aux_kernel");

InputParameters
ElementIDOutputAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Action for copying extra element IDs into auxiliary variables for output.");
  return params;
}

ElementIDOutputAction::ElementIDOutputAction(const InputParameters & params) : Action(params) {}

void
ElementIDOutputAction::act()
{
  // Do nothing if the application does not have output
  if (!_app.actionWarehouse().hasActions("add_output"))
    return;

  if (_current_task == "add_aux_kernel")
  {
    const auto & output_actions = _app.actionWarehouse().getActionListByName("add_output");
    for (const auto & act : output_actions)
    {
      // Extract the Output action
      AddOutputAction * action = dynamic_cast<AddOutputAction *>(act);
      if (!action)
        continue;

      InputParameters & params = action->getObjectParams();
      if (params.isParamValid("output_extra_element_ids") &&
          params.get<bool>("output_extra_element_ids"))
      {
        bool has_element_id_names = params.isParamValid("extra_element_ids_to_output");
        std::vector<std::string> element_id_names;
        if (has_element_id_names)
          element_id_names = params.get<std::vector<std::string>>("extra_element_ids_to_output");

        auto var_params = _factory.getValidParams("MooseVariableConstMonomial");
        auto kernel_params = _factory.getValidParams("ExtraElementIDAux");
        kernel_params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
        for (unsigned int i = 0; i < _mesh->getMesh().n_elem_integers(); ++i)
        {
          auto & var_name = _mesh->getMesh().get_elem_integer_name(i);
          if (!has_element_id_names ||
              (std::find(element_id_names.begin(), element_id_names.end(), var_name) !=
               element_id_names.end()))
          {
            // Create aux variables based on the extra element id name
            _problem->addAuxVariable("MooseVariableConstMonomial", var_name, var_params);

            // Create aux kernels based on the extra element id name
            kernel_params.set<AuxVariableName>("variable") = var_name;
            kernel_params.set<std::vector<ExtraElementIDName>>("extra_id_name") = {var_name};
            _problem->addAuxKernel("ExtraElementIDAux", "_output_" + var_name, kernel_params);
          }
        }
      }
    }
  }
}
